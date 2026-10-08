# BridgeSpoofer - Lilu plugin build (no Xcode project needed)
#
#   ./bootstrap.sh        # once: fetches Lilu.kext SDK + MacKernelSDK
#   make                  # release build -> build/Release/BridgeSpoofer.kext
#   make CONFIG=Debug     # debug build (DBGLOG enabled, use -bridgedbg)
#   make zip              # zipped kext in build/

PRODUCT   := BridgeSpoofer
VERSION   := 1.0.0
BUNDLE_ID := org.albert-mueller.BridgeSpoofer
CONFIG    ?= Release

LILU_SDK  := Lilu.kext/Contents/Resources
KSDK      := MacKernelSDK
BUILD     := build/$(CONFIG)
OBJ       := $(BUILD)/obj
KEXT      := $(BUILD)/$(PRODUCT).kext

CXX       := xcrun clang++
CC        := xcrun clang
ARCH      := -arch x86_64
MINOS     := -mmacosx-version-min=10.13

DEFS := -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT \
        -DPRODUCT_NAME=$(PRODUCT) -DMODULE_VERSION=$(VERSION)

ifeq ($(CONFIG),Debug)
  DEFS += -DDEBUG
  OPT  := -O0 -g
else
  OPT  := -Os -g
endif

INCS := -nostdinc -isystem $(KSDK)/Headers -I$(LILU_SDK) -IBridgeSpooferSrc

CFLAGS   := $(ARCH) $(MINOS) $(OPT) $(DEFS) $(INCS) -mkernel -fno-builtin -fno-common \
            -fno-stack-protector -Wall -Wno-unused-parameter -Wno-format
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fapple-kext -fno-exceptions -fno-rtti -nostdinc++

CC_KEXT  := $(shell xcrun clang -print-resource-dir 2>/dev/null)/lib/darwin/libclang_rt.cc_kext.a
LDFLAGS  := $(ARCH) $(MINOS) -nostdlib -Xlinker -kext \
            $(KSDK)/Library/x86_64/libkmod.a $(CC_KEXT)

SRCS := BridgeSpooferSrc/main.cpp BridgeSpooferSrc/BridgeSpoofer.cpp $(LILU_SDK)/Library/plugin_start.cpp
OBJS := $(patsubst %.cpp,$(OBJ)/%.o,$(notdir $(SRCS))) $(OBJ)/kmod_info.o

vpath %.cpp BridgeSpooferSrc $(LILU_SDK)/Library

.PHONY: all clean zip check

all: check $(KEXT)

check:
	@test -f $(LILU_SDK)/Headers/kern_api.hpp || { echo "Lilu SDK missing - run ./bootstrap.sh"; exit 1; }
	@test -f $(KSDK)/Library/x86_64/libkmod.a || { echo "MacKernelSDK missing - run ./bootstrap.sh"; exit 1; }
	@test -f "$(CC_KEXT)" || { echo "libclang_rt.cc_kext.a not found - install Xcode"; exit 1; }

$(OBJ)/%.o: %.cpp | $(OBJ)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Equivalent of Xcode's generated <product>_info.c
$(OBJ)/kmod_info.c: | $(OBJ)
	@printf '%s\n' \
	  '#include <mach/mach_types.h>' \
	  'extern kern_return_t _start(kmod_info_t *, void *);' \
	  'extern kern_return_t _stop(kmod_info_t *, void *);' \
	  'extern kern_return_t $(PRODUCT)_kern_start(kmod_info_t *, void *);' \
	  'extern kern_return_t $(PRODUCT)_kern_stop(kmod_info_t *, void *);' \
	  '__attribute__((visibility("default"))) KMOD_EXPLICIT_DECL($(BUNDLE_ID), "$(VERSION)", _start, _stop)' \
	  '__private_extern__ kmod_start_func_t *_realmain = $(PRODUCT)_kern_start;' \
	  '__private_extern__ kmod_stop_func_t *_antimain = $(PRODUCT)_kern_stop;' \
	  '__private_extern__ int _kext_apple_cc = __APPLE_CC__;' > $@

$(OBJ)/kmod_info.o: $(OBJ)/kmod_info.c
	$(CC) $(CFLAGS) -c $< -o $@

$(KEXT): $(OBJS) Info.plist
	mkdir -p $(KEXT)/Contents/MacOS
	$(CXX) $(LDFLAGS) $(OBJS) -o $(KEXT)/Contents/MacOS/$(PRODUCT)
	cp Info.plist $(KEXT)/Contents/Info.plist
	@echo "Built $(KEXT)"

$(OBJ):
	mkdir -p $@

zip: all
	cd $(BUILD) && rm -f ../$(PRODUCT)-$(VERSION)-$(CONFIG).zip && \
	  zip -qr ../$(PRODUCT)-$(VERSION)-$(CONFIG).zip $(PRODUCT).kext
	@echo "Created build/$(PRODUCT)-$(VERSION)-$(CONFIG).zip"

clean:
	rm -rf build
