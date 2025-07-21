#include "Build/Platform.h"
#include "Util/ErrorMessage.h"

using namespace stride;

namespace platform {

JUCE_IMPLEMENT_SINGLETON (PlatformManager)

std::string getProductStringFromEnum(PlatformEnum platformEnum)
{
    switch(platformEnum) {    
    case PlatformEnum::TEENSY  : return std::string("STRIDE");
    case PlatformEnum::RASPPI4 : return std::string("STRIDE MKII");
    case PlatformEnum::INVALID :
    default : return std::string("INVALID");
    }
}

// PLATFORM BASE
PlatformBase::PlatformBase(PlatformEnum platformEnum)
: m_platformEnum(platformEnum)
{
    
}

PlatformConfig& PlatformBase::getConfig() { return m_platformConfig; }

std::string PlatformBase::getProductName() { return m_platformConfig.productName; }

std::string PlatformBase::getMcuType() { return m_platformConfig.mcuTypeName; }

// PLATFORM MANAGER
void PlatformManager::addPlatform(std::shared_ptr<PlatformBase> platformPtr)
{
    if (!platformPtr) {
        errorMessage("PlatformManager::addPlatform(): platformPtr is nullptr");
        return;
    }
    m_platformsVec.emplace_back(platformPtr);
}

std::shared_ptr<PlatformBase> PlatformManager::getCurrentPlatform()
{
    return m_activePlatform;   
}

PlatformEnum PlatformManager::getCurrentPlatformType()
{
    if (m_activePlatform) {
        return m_activePlatform->getPlatformType();
    } else {
        return PlatformEnum::INVALID;
    }
}

std::string PlatformManager::getCurrentPlatformProductName()
{
    if (m_activePlatform) {
        return m_activePlatform->getProductName();
    } else {
        return std::string("INVALID");
    }
}

bool PlatformManager::setCurrentPlatform(PlatformEnum platformEnum)
{
    for (auto& platformPtr : m_platformsVec) {
        if (platformPtr) {
            if (platformPtr->getPlatformType() == platformEnum) {
                m_activePlatform = platformPtr;
                return true;
            }
        }
    }
    return false;
}

bool PlatformManager::isPlatformValid(PlatformEnum platformEnum)
{
    for (auto& platformPtr : m_platformsVec) {
        if (platformPtr) {
            if (platformPtr->getPlatformType() == platformEnum) {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::shared_ptr<PlatformBase>> PlatformManager::getPlatformsVec()
{
    return m_platformsVec;
}

std::string PlatformBase::getAuxFunctions()
{
    return std::string();
}

std::string PlatformBase::getLinkerFileName()
{
    return std::string("linker.ld");
}

std::vector<std::string> PlatformBase::getExtraIncludeLibs()
{
    std::vector<std::string> empty;
    return empty;
}

std::string PlatformBase::getCommonMakefileAssigns()
{
    std::string commonStr;

    commonStr += "\
TOOL_PREFIX=$(COMPILER_PATH)/$(COMPILER_FILENAME)\n\
CC      = $(TOOL_PREFIX)gcc\n\
CXX     = $(TOOL_PREFIX)g++\n\
LD      = $(TOOL_PREFIX)ld\n\
OBJCOPY = $(TOOL_PREFIX)objcopy\n\
OBJDUMP = $(TOOL_PREFIX)objdump\n\
\n\
CPPFLAGS += -c -Wall -fsigned-char $(COMMON_FLAGS)\n\
CPPFLAGS += -ffunction-sections -fdata-sections -fno-exceptions\n\
CPPFLAGS += -Wno-error=narrowing\n\
\n\
CPPFLAGS += -DPROCESS_SERIAL_MIDI\n\
CPPFLAGS += -DAUDIO_BLOCK_SAMPLES=128 -DAUDIO_SAMPLE_RATE_EXACT=48000.0f\n\
CFLAGS   += -std=gnu99\n\
CXXFLAGS += -std=gnu++17\n\
\n\
ifeq ($(AVALON_REV),2)\n\
CPPFLAGS += -DAVALON_REV2\n\
endif\n\
ifdef PROCESS_USB_MIDI\n\
CPPFLAGS += -DPROCESS_USB_MIDI\n\
endif\n\
ifdef PROCESS_SERIAL_MIDI\n\
CPPFLAGS += -DPROCESS_SERIAL_MIDI\n\
endif\n\
\n\
CPPFLAGS += -I./include -I../efx\n\
INCLUDE_DIRS_LIST += arm_math sysPlatform globalCompat Stride Avalon\n\
INCLUDE_DIRS += $(addprefix -I../efx/, $(EFX_DIR_LIST)) \n\
\n\
LDFLAGS  += -L./lib -L../efx\n\
LD_FILE  += -T./linker.ld\n\
RELEASEFLAGS = -s -fvisibility=hidden -O2 -D NDEBUG\n\
OBJ_FILES = $(addsuffix .o, $(SRC_FILE_LIST))\n\
SYS_STAT_LIBS += $(addprefix -l:, $(EFX_LIST))\n\
CORE_LIBS = -l:$(CORE_FILENAME)\n\
";

    return commonStr;
}

}
