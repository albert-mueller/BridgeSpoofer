#include <Headers/plugin_start.hpp>
#include <Headers/kern_api.hpp>
#include "BridgeSpoofer.hpp"

static BridgeSpoofer bridgeSpoofer;

static const char *bootargOff[]   { "-bridgeoff" };
static const char *bootargDebug[] { "-bridgedbg" };
static const char *bootargBeta[]  { "-bridgebeta" };

// Lilu plugin configuration (field order must match PluginConfiguration in plugin_start.hpp)
PluginConfiguration ADDPR(config) {
    xStringify(PRODUCT_NAME),
    parseModuleVersion("1.0.0"),
    LiluAPI::AllowNormal | LiluAPI::AllowInstallerRecovery | LiluAPI::AllowSafeMode,
    bootargOff,
    arrsize(bootargOff),
    bootargDebug,
    arrsize(bootargDebug),
    bootargBeta,
    arrsize(bootargBeta),
    KernelVersion::HighSierra,   // first macOS on T2 Macs
    KernelVersion::Tahoe,
    []() {
        bridgeSpoofer.init();
    }
};
