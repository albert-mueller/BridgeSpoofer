#include "BridgeSpoofer.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/kern_util.hpp>

mach_vm_address_t BridgeSpoof::orgExternalMethod {0};

// User client classes we want to inspect (verify the exact names with `ioreg -l` on the target machine)
static const char *targetClients[] {
    "AppleEmbeddedOSSupportClient",
    "AppleEmbeddedDeviceClient",
};

static bool isTargetClient(const IOUserClient *client) {
    if (!client)
        return false;
    auto meta = client->getMetaClass();
    if (!meta)
        return false;
    const char *name = meta->getClassName();
    if (!name)
        return false;
    for (auto targetName : targetClients)
        if (strcmp(name, targetName) == 0)
            return true;
    return false;
}

void BridgeSpoof::init() {
    DBGLOG("bridgespoof", "Initializing BridgeSpoofer plugin");

    // Kernel functions can only be routed once Lilu's patcher is ready, not directly in pluginStart
    lilu.onPatcherLoadForce([](void *user, KernelPatcher &patcher) {
        static_cast<BridgeSpoof *>(user)->processKernel(patcher);
    }, this);
}

void BridgeSpoof::processKernel(KernelPatcher &patcher) {
    // IOUserClient::externalMethod(uint32_t, IOExternalMethodArguments*, IOExternalMethodDispatch*, OSObject*, void*)
    KernelPatcher::RouteRequest request {
        "__ZN12IOUserClient14externalMethodEjP25IOExternalMethodArgumentsP24IOExternalMethodDispatchP8OSObjectPv",
        ourExternalMethod, orgExternalMethod
    };

    if (patcher.routeMultiple(KernelPatcher::KernelID, &request, 1)) {
        SYSLOG("bridgespoof", "Successfully routed IOUserClient::externalMethod");
    } else {
        SYSLOG("bridgespoof", "Failed to route IOUserClient::externalMethod (error %d)", static_cast<int>(patcher.getError()));
        patcher.clearError();
    }
}

IOReturn BridgeSpoof::ourExternalMethod(IOUserClient *client, uint32_t selector,
                                          IOExternalMethodArguments *arguments,
                                          IOExternalMethodDispatch *dispatch,
                                          OSObject *target, void *reference) {
    // 1. Let the original driver method run first so the output data exists
    IOReturn result = FunctionCast(ourExternalMethod, orgExternalMethod)(
        client, selector, arguments, dispatch, target, reference
    );

    // 2. This hook sees EVERY user client call in the system, so leave as fast as possible
    //    for everything that isn't ours. No logging here, or the log gets flooded.
    if (!isTargetClient(client))
        return result;

    if (result != kIOReturnSuccess) {
        DBGLOG("bridgespoof", "T2 client selector %u failed: 0x%x", selector, result);
        return result;
    }

    if (!arguments) {
        DBGLOG("bridgespoof", "T2 client selector %u: no arguments", selector);
        return result;
    }

    if (arguments->structureOutput && arguments->structureOutputSize > 0) {
        DBGLOG("bridgespoof", "T2 client selector %u: inline output, %u bytes",
               selector, arguments->structureOutputSize);
        // Inspect/modify arguments->structureOutput here
    } else if (arguments->structureOutputDescriptor) {
        // Large outputs (> 4 KB) arrive through a memory descriptor instead of structureOutput
        DBGLOG("bridgespoof", "T2 client selector %u: descriptor output, %u bytes",
               selector, arguments->structureOutputDescriptorSize);
    } else {
        DBGLOG("bridgespoof", "T2 client selector %u: no structure output", selector);
    }

    return result;
}
