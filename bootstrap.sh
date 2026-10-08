#!/bin/bash
#
# No-Xcode bootstrap: fetches the PREBUILT Lilu SDK (Lilu.kext DEBUG release,
# which contains Headers/ and Library/plugin_start.cpp) and MacKernelSDK.
# Needs only curl, unzip and git - no Xcode, no xcodebuild.
#
set -e
cd "$(dirname "$0")"

if [ -f Lilu.kext/Contents/Resources/Headers/kern_api.hpp ] && [ -d MacKernelSDK ]; then
  echo "SDKs already present."
  exit 0
fi

if [ -f ../Spoof-VMM-T2-Stage2/Lilu.kext/Contents/Resources/Headers/kern_api.hpp ] && [ -d ../Spoof-VMM-T2-Stage2/MacKernelSDK ]; then
  echo "Found SDKs in sibling Spoof-VMM-T2-Stage2, copying..."
  cp -R ../Spoof-VMM-T2-Stage2/Lilu.kext .
  cp -R ../Spoof-VMM-T2-Stage2/MacKernelSDK .
  echo "SDKs successfully set up."
  exit 0
fi

# Determine the latest Lilu tag via the releases/latest web redirect.
# (Uses the normal redirect, not the API, so it isn't API-rate-limited.)
# Override by running:  TAG=1.7.0 ./bootstrap.sh
if [ -z "$TAG" ]; then
  TAG=$(curl -fsSLI -o /dev/null -w '%{url_effective}' \
        https://github.com/acidanthera/Lilu/releases/latest | sed 's#.*/tag/##')
fi
if [ -z "$TAG" ]; then
  echo "ERROR: could not determine the latest Lilu version."
  echo "Find it at https://github.com/acidanthera/Lilu/releases and run:"
  echo "  TAG=<version> ./bootstrap.sh   (e.g. TAG=1.7.0 ./bootstrap.sh)"
  exit 1
fi
echo "-> Lilu ${TAG} (prebuilt DEBUG)"

URL="https://github.com/acidanthera/Lilu/releases/download/${TAG}/Lilu-${TAG}-DEBUG.zip"
if ! curl -fsSL -o Lilu.zip "$URL"; then
  echo "ERROR: download failed: $URL"
  exit 1
fi

rm -rf Lilu.kext tmp_lilu
mkdir tmp_lilu
( cd tmp_lilu && unzip -q ../Lilu.zip )
if [ ! -d tmp_lilu/Lilu.kext ]; then
  echo "ERROR: Lilu.kext not found in the downloaded archive."
  exit 1
fi
mv tmp_lilu/Lilu.kext .
rm -rf tmp_lilu Lilu.zip

if [ ! -d MacKernelSDK ]; then
  echo "-> MacKernelSDK"
  git clone -q --depth=1 https://github.com/acidanthera/MacKernelSDK.git
fi

# Sanity: make sure the two files the Makefile needs are present.
if [ ! -f Lilu.kext/Contents/Resources/Headers/kern_api.hpp ]; then
  echo "ERROR: SDK headers missing after extract."
  exit 1
fi
if [ ! -f Lilu.kext/Contents/Resources/Library/plugin_start.cpp ]; then
  echo "WARNING: plugin_start.cpp not in the prebuilt SDK's Library folder."
  echo "Fetching it from the Lilu source tree as a fallback..."
  mkdir -p Lilu.kext/Contents/Resources/Library
  curl -fsSL -o Lilu.kext/Contents/Resources/Library/plugin_start.cpp \
    "https://raw.githubusercontent.com/acidanthera/Lilu/${TAG}/Lilu/Library/plugin_start.cpp" \
    || { echo "ERROR: could not fetch plugin_start.cpp"; exit 1; }
fi

echo "Done. Now run: make"
