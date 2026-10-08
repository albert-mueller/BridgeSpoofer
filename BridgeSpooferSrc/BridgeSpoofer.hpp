#ifndef BRIDGESPOOFER_HPP
#define BRIDGESPOOFER_HPP

#include <Headers/plugin_start.hpp>
#include <IOKit/IOUserClient.h>
#include <IOKit/IOExternalMethodArguments.h>

class BridgeSpoofer {
public:
    void init();
    void deinit();

    // Trampoline pointer for the original kernel function
    static mach_vm_address_t orgExternalMethod;

    // Hooked function handler
    static IOReturn ourExternalMethod(IOUserClient *client, uint32_t selector,
                                      IOExternalMethodArguments *arguments,
                                      IOExternalMethodDispatch *dispatch,
                                      OSObject *target, void *reference);
};

#endif // BRIDGESPOOFER_HPP
