# BridgeSpoofer

A Lilu kernel extension to intercept and modify `IOUserClient::externalMethod` calls targeting Apple Embedded OS support services (`AppleEmbeddedOSSupportClient`, `AppleEmbeddedDeviceClient`) on T2 Macs.

## Requirements

- macOS 10.13 or newer build host
- Xcode command line tools (`clang`, `clang++`)
- Lilu 1.7.0+ (downloaded automatically by `bootstrap.sh`)
- MacKernelSDK (downloaded automatically by `bootstrap.sh`)

## Building

```bash
# 1. Fetch Lilu SDK & MacKernelSDK (one-time setup)
./bootstrap.sh

# 2. Build release kext
make

# Or build debug kext with DBGLOG enabled:
make CONFIG=Debug

# Package zip artifact:
make zip
```

The built kext will be located in `build/Release/BridgeSpoofer.kext` (or `build/Debug/BridgeSpoofer.kext`).

## Boot Arguments

- `-bridgeoff`: Disable BridgeSpoofer
- `-bridgedbg`: Enable debug logging (requires Debug build)
- `-bridgebeta`: Enable on unsupported macOS versions