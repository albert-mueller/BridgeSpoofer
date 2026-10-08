#ifndef BRIDGESPOOFER_HPP
#define BRIDGESPOOFER_HPP

#include <Headers/kern_patcher.hpp>
#include <IOKit/IOUserClient.h>   // also defines IOExternalMethodArguments / IOExternalMethodDispatch

class BridgeSpoofer {
public:
    void init();

private:
    // Called by Lilu once the kernel patcher is ready
    void processKernel(KernelPatcher &patcher);

    // Trampoline to the original IOUserClient::externalMethod
    static mach_vm_address_t orgExternalMethod;

    // Hook. externalMethod is a C++ member function, so 'this' (the client) comes first.
    static IOReturn ourExternalMethod(IOUserClient *client, uint32_t selector,
                                      IOExternalMethodArguments *arguments,
                                      IOExternalMethodDispatch *dispatch,
                                      OSObject *target, void *reference);
};

#endif // BRIDGESPOOFER_HPP
