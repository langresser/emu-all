ENV := ios
CROSS_COMPILE := 1

ifndef target
 target = $(metadata_exec)
endif

ifndef targetDir
 ifdef O_RELEASE
  targetDir := target/iOS/bin-release
 else
  targetDir := target/iOS/bin-debug
 endif
endif

config_compiler := clang
ifeq ($(origin CC), default)
 CC := clang
endif
include $(currPath)/clang.mk

ifdef ios_linkerPath
 # define an alternate PATH when linking
 LD = PATH=$(ios_linkerPath):$(PATH) $(CC)
endif

ifdef RELEASE
 COMPILE_FLAGS += -DNS_BLOCK_ASSERTIONS
endif

 # base engine code needs at least iOS 3.1
minIOSVer := 3.1
IOS_SDK ?= 6.0
ifeq ($(ARCH),x86)
 IOS_SYSROOT ?= $(shell xcode-select --print-path)/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator$(IOS_SDK).sdk
 IOS_FLAGS = -isysroot $(IOS_SYSROOT) -mios-simulator-version-min=$(minIOSVer) -fobjc-abi-version=2 -fobjc-legacy-dispatch
else
 IOS_SYSROOT ?= $(shell xcode-select --print-path)/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS$(IOS_SDK).sdk
 IOS_FLAGS = -isysroot $(IOS_SYSROOT) -miphoneos-version-min=$(minIOSVer)
endif
CPPFLAGS += $(IOS_FLAGS)
LDFLAGS += $(IOS_FLAGS)

ifndef ios_noDeadStrip
 LDFLAGS += -dead_strip
endif
ifdef RELEASE
 LDFLAGS += -Wl,-S,-x,-dead_strip_dylibs
endif
WHOLE_PROGRAM_CFLAGS := -fipa-pta -fwhole-program

CPPFLAGS += -I$(IMAGINE_PATH)/bundle/darwin-iOS/include
noDoubleFloat=1

# clang SVN doesn't seem to handle ASM properly so use as directly
AS := as
ASMFLAGS :=
