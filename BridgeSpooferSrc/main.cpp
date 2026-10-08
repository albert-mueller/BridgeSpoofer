#include <Headers/plugin_start.hpp>
#include "BridgeSpoofer.hpp"

// Define plugin metadata configuration required by Lilu
PluginConfiguration ADDPR(config) = {
    xStringify(PRODUCT_NAME),
    "1.0.0",
    BootArgs::getBootArgVal("-bridgeoff"),
    -1,
    CPU_GEN_ALL,
    nullptr,
    0,
    nullptr,
    0,
    nullptr
};

static BridgeSpoofer bridgeSpooferInstance;

void pluginStart() {
    DBGLOG("bridgespoof", "pluginStart loaded");
    bridgeSpooferInstance.init();
}

void pluginStop() {
    DBGLOG("bridgespoof", "pluginStop unloaded");
    bridgeSpooferInstance.deinit();
}
