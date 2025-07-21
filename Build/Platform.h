#pragma once

#include <string>
#include <vector>
#include <memory>

#include <JuceHeader.h>

namespace platform {

enum class PlatformEnum : unsigned {
    INVALID = 0,
    FIRST_PLATFORM = 1,
    TEENSY  = 1,
    RASPPI4 = 2,
    NUM_VALID_PLATFORMS = RASPPI4
};

std::string getProductStringFromEnum(PlatformEnum platformEnum);

struct PlatformConfig {
    std::string TOOLCHAIN_PREFIX;
    std::string TOOLCHAIN_VERSION;
    std::string BUILD_OUTPUT_BINARY;
    std::string PROGRAMMING_FILE;
    std::string LINKER_FILENAME;
    std::string AVALON_AUX_FUNCTIONS;

    float BASE_CPU_LOAD_PERCENT;
    float BASE_RAM0_LOAD_PERCENT;
    float BASE_RAM1_LOAD_PERCENT;
    float BASE_AUDIO_BUFFERS;

    float PROGRAM_FLASH_MAX_SIZE;
    float PROGRAM_RAM_SIZE;
    float PROGRAM_RAM0_SAFETY_RATIO;
    float PROGRAM_RAM1_SAFETY_RATIO;
    float CPU_SAFETY_THRESHOLD;
    float COMMON_SAFETY_RATIO;

    std::string productName;
    std::string mcuTypeName;
};

class PlatformBase {
public:
    struct Flags {
        bool noPrintf       = true;
        bool isDebug        = false;
        bool enableFastMath = true;
        bool enableO3       = true;
    };

    PlatformBase() = delete;
    PlatformBase(PlatformEnum platformEnum);

    virtual void configPlatform() = 0;  // child classes use this to configure the platform settings
    PlatformConfig& getConfig();

    virtual std::string getProductName();
    virtual std::string getMcuType();

    PlatformEnum getPlatformType() { return m_platformEnum; }
    virtual std::string getCommonMakefileAssigns();

    virtual int unzipBuildTools(const std::string& toolsDirectory) = 0;

    virtual std::string getLinkerFile() = 0;
    virtual std::string getLinkerFileName();
    virtual std::string getMakefile() = 0;

    virtual const char* getCoreIncludesZip() = 0;
    virtual size_t      getCoreIncludesZipSize() = 0;
    virtual const char* getCoreLibsZip() = 0;
    virtual size_t      getCoreLibsZipSize() = 0;

    virtual std::string getAuxFunctions();


    virtual std::string createTestMakefile(const std::string& toolsDirectory, const std::string& libsDirectory,
        const std::string& datFilename, const std::string& testAppName, const std::string& irDataName,
        const std::vector<std::string>& includeDirectoriesVec
        ) = 0;

    virtual std::string getEfxMakefileInc(const Flags compilerFlags, const std::string& cppFlags) = 0;
    virtual std::vector<std::string> getExtraIncludeLibs();

    virtual size_t getFlashMaxSize() = 0;

    virtual bool isProgramRamValid(const std::string& toolsDirectory, const std::string& programDir, const std::string& programName,
        float& ram0Min, float& ram1Min) = 0;
    virtual bool isProgramFlashValid(const std::string& toolsDirectory, const std::string& programDir, const std::string& programName) = 0;

    virtual int  loadBinaryFile(const std::string& binaryFilePath) = 0;  // returns the binary programming size of the file
    virtual int  openUsb() = 0;
    virtual int  programDevice()  = 0;
    virtual void requestProgramThreadExit() = 0;
    virtual float getProgrammingProgress() = 0;
    virtual bool isEraseDone() = 0;
protected:
    PlatformEnum m_platformEnum = PlatformEnum::INVALID;
    PlatformConfig m_platformConfig;
};

class PlatformManager {
public:
    void addPlatform(std::shared_ptr<PlatformBase> platformPtr);
    std::shared_ptr<PlatformBase> getCurrentPlatform();
    bool setCurrentPlatform(PlatformEnum platformEnum);
    bool isPlatformValid(PlatformEnum platformEnum);

    PlatformEnum getCurrentPlatformType();
    std::string getCurrentPlatformProductName();

    std::vector<std::shared_ptr<PlatformBase>> getPlatformsVec();

    JUCE_DECLARE_SINGLETON (PlatformManager, false)
private:
    std::vector<std::shared_ptr<PlatformBase>> m_platformsVec;
    std::shared_ptr<PlatformBase> m_activePlatform;    
};

}

#if defined(RASPPI)
#include "Build/PlatformRpi4.h"
#endif

#if defined(TEENSY_MICROMOD)
#include "Build/PlatformTeensy.h"
#endif
