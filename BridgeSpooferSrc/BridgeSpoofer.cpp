#include "BridgeSpoofer.hpp"
#include <Headers/kern_api.hpp>
#include <Headers/kern_util.hpp>

// Initialize static member variables
mach_vm_address_t BridgeSpoofer::orgExternalMethod = 0;

void BridgeSpoofer::init() {
    DBGLOG("bridgespoof", "Initializing BridgeSpoofer plugin for j140kap / MacBookAir8,1");

    // XNU Kernel symbol for IOUserClient::externalMethod
    const char *symbol = "__ZN12IOUserClient14externalMethodEjP25IOExternalMethodArgumentsP20IOExternalMethodDispatchP8OSObjectPv";

    if (korg::routeFunction(kernel_function_t(symbol), (void *)ourExternalMethod, (void **)&orgExternalMethod)) {
        SYSLOG("bridgespoof", "Successfully routed IOUserClient::externalMethod.");
    } else {
        SYSLOG("bridgespoof", "Failed to route IOUserClient::externalMethod.");
    }
}

void BridgeSpoofer::deinit() {
    DBGLOG("bridgespoof", "Deinitializing BridgeSpoofer plugin.");
}

IOReturn BridgeSpoofer::ourExternalMethod(IOUserClient *client, uint32_t selector,
                                          IOExternalMethodArguments *arguments,
                                          IOExternalMethodDispatch *dispatch,
                                          OSObject *target, void *reference) {
    
    // 1. Let the original driver method run first to generate the status data structure
    IOReturn result = FunctionCast(ourExternalMethod, orgExternalMethod)(
        client, selector, arguments, dispatch, target, reference
    );

    // 2. If the call succeeded and output buffers are present, check the client context
    if (result == kIOReturnSuccess && arguments && arguments->structureOutput) {
        if (client) {
            const OSSymbol *className = client->copyClassName();
            if (className) {
                // Target the T2 / Apple Embedded OS communication clients specifically
                if (className->isEqualTo("AppleEmbeddedOSSupportClient") || 
                    className->isEqualTo("AppleEmbeddedDeviceClient")) {
                    
                    DBGLOG("bridgespoof", "Intercepted target T2 client request. Selector: %u", selector);
                    
                    // Note: You can inspect/modify arguments->structureOutput and 
                    // arguments->structureOutputSize here before it returns to remotectl.
                }
                className->release();
            }
        }
    }
    else {
        SYSLOG("Call/output vuffers are not present"); // I'm writing in C++ for the first time without AI, but I want to show that there needs to be some error handling. Please verify the syntax and fix if something's wrong.
    }

    return result;
}
