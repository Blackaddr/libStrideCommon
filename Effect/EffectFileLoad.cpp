/*
 * EffectFileLoad.cpp
 *
 *  Created on: Jan. 9, 2021
 *      Author: blackaddr
 */
#include <JuceHeader.h>
#include <algorithm>  // for std::find
#include <csignal>
#include <csetjmp>
#include <cmath>
#include <string>
#include <vector>
#include <mutex>

#include "Util/CommonDefs.h"
#include "Util/ErrorMessage.h"
#include "Util/ErrorMessageWindow.h"
#include "Util/StringUtil.h"

#include "Resources/BinaryIcons.h"
#include "Resources/BinaryIconsSize.h"
#include "Build/BuildCommon.h"
#include "Util/FileUtil.h"
#include "Util/Graphics.h" // DefaultControlImages class
#include "Effect/EffectFileLoad.h"

using namespace juce;

namespace stride {

static jmp_buf gBuffer;        // A buffer to hold info on where to jump to
static void (*gPrevHandler[NSIG])(int);
static std::mutex g_mutex;

static void catch_signal(int signalNumber)
{
    //signal(SIGINT, SIG_DFL);          // Switch to default handling
    for (int i=0; i < NSIG; i++) {
        signal(i, catch_signal);       // Reactivate this handler.
    }

    longjmp             // Jump back into the normal flow of the program
    (
        gBuffer,        // using this context to say where to jump to
        signalNumber    // and passing back the value of the signal.
    );
}

constexpr unsigned I2S_INPUT_INDEX  = 0;
constexpr unsigned I2S_OUTPUT_INDEX = 1;
constexpr unsigned USB_INPUT_INDEX  = 2;
constexpr unsigned USB_OUTPUT_INDEX = 3;
constexpr unsigned SHUNT_INDEX      = 4;

bool isStringInArray(const std::string& searchString, const std::vector<std::string>& vec);

Image getPngFromZip(const std::string& filename, ZipFile& zipFile);

void loadInputsOutputs(std::vector<std::shared_ptr<EffectFileData>> &effectFileDataVec)
{
    // Add the I2S Input
    {
        auto ioFileDataPtr                = std::make_shared<EffectFileData>();
        ioFileDataPtr->isInputOutput      = true;
        ioFileDataPtr->isSingleton        = true;
        ioFileDataPtr->processMidi        = false;
        ioFileDataPtr->isDataPak          = false;
        ioFileDataPtr->audioStreamType    = AudioStreamType::INT16;
        ioFileDataPtr->company            = COMPANY_NAME;
        ioFileDataPtr->effectName         = "ANALOG INPUT";
        ioFileDataPtr->effectCategory     = "Input";
        ioFileDataPtr->effectCategoryEnum = EffectCategory::INPUT;
        ioFileDataPtr->effectDescription  = "Channel 0/1 are the LEFT/RIGHT stereo jack inputs";
        ioFileDataPtr->numControls        = 0;
        ioFileDataPtr->numInputs          = 0;
        ioFileDataPtr->numOutputs         = 2;
        ioFileDataPtr->libraryName        = "";
#if defined(AVALON_REV2)
        ioFileDataPtr->cppClass           = "SysAudioInputI2S";
#elif defined(AVALON_REV1)
        ioFileDataPtr->cppClass           = "SysAudioInputI2S";
#else
#error "Avalon REV not defined"
#endif

        ioFileDataPtr->cppInstBase        = "i2sIn";
        ioFileDataPtr->cpuUsage           = 0.5f;
        ioFileDataPtr->ram0Usage          = 0.0f;
        ioFileDataPtr->ram1Usage          = 0.0f;
        ioFileDataPtr->writableBuffers    = 4; // possible for one new set of stereo buffers and one in progress

        auto image = ImageCache::getFromMemory(BinaryIcons::inputTrsPlug_png, BinaryIcons::inputTrsPlug_pngSize);
        std::shared_ptr<EffectImage> imagePtr = std::make_shared<EffectImage>(image);
        ioFileDataPtr->setImagePtr(imagePtr);

        ioFileDataPtr->effectIndexId = I2S_INPUT_INDEX;
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            effectFileDataVec.push_back(ioFileDataPtr);
        }

    }

    // Add the I2S Output
    {
        auto ioFileDataPtr                = std::make_shared<EffectFileData>();
        ioFileDataPtr->isInputOutput      = true;
        ioFileDataPtr->isSingleton        = true;
        ioFileDataPtr->processMidi        = false;
        ioFileDataPtr->isDataPak          = false;
        ioFileDataPtr->audioStreamType    = AudioStreamType::INT16;
        ioFileDataPtr->company            = COMPANY_NAME;
        ioFileDataPtr->effectName         = "ANALOG OUTPUT";
        ioFileDataPtr->effectCategory     = "Output";
        ioFileDataPtr->effectCategoryEnum = EffectCategory::OUTPUT;
        ioFileDataPtr->effectDescription  = "Channel 0/1 are the LEFT/RIGHT stereo jack outputs";
        ioFileDataPtr->numControls        = 0;
        ioFileDataPtr->numInputs          = 2;
        ioFileDataPtr->numOutputs         = 0;
        ioFileDataPtr->libraryName        = "";
        ioFileDataPtr->cppClass           = "Avalon::AudioEffectOutputLevel";
        ioFileDataPtr->cppInstBase        = "outputLevel";
        ioFileDataPtr->cpuUsage           = 0.5f;
        ioFileDataPtr->ram0Usage          = 0.0f;
        ioFileDataPtr->ram1Usage          = 0.0f;
        ioFileDataPtr->writableBuffers    = 4; // possible for one new set of stereo buffers and one in progress

        auto image = ImageCache::getFromMemory(BinaryIcons::outputTrsPlug_png, BinaryIcons::outputTrsPlug_pngSize);
        std::shared_ptr<EffectImage> imagePtr = std::make_shared<EffectImage>(image);
        ioFileDataPtr->setImagePtr(imagePtr);

        ioFileDataPtr->effectIndexId = I2S_OUTPUT_INDEX;
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            effectFileDataVec.push_back(ioFileDataPtr);
        }
    }

    // Add the USB Input
    {
        auto ioFileDataPtr                = std::make_shared<EffectFileData>();
        ioFileDataPtr->isInputOutput      = true;
        ioFileDataPtr->isSingleton        = true;
        ioFileDataPtr->processMidi        = false;
        ioFileDataPtr->isDataPak          = false;
        ioFileDataPtr->audioStreamType    = AudioStreamType::INT16;
        ioFileDataPtr->company            = COMPANY_NAME;
        ioFileDataPtr->effectName         = "USB INPUT";
        ioFileDataPtr->effectCategory     = "Input";
        ioFileDataPtr->effectCategoryEnum = EffectCategory::INPUT;
        ioFileDataPtr->effectDescription  = "Channel 0/1 are the LEFT/RIGHT stereo USB inputs";
        ioFileDataPtr->numControls        = 0;
        ioFileDataPtr->numInputs          = 0;
        ioFileDataPtr->numOutputs         = 2;
        ioFileDataPtr->libraryName        = "";
        ioFileDataPtr->cppClass           = "SysAudioInputUsb";
        ioFileDataPtr->cppInstBase        = "usbIn";
        ioFileDataPtr->cpuUsage           = 0.5f;
        ioFileDataPtr->ram0Usage          = 0.0f;
        ioFileDataPtr->ram1Usage          = 0.0f;
        ioFileDataPtr->writableBuffers    = 4; // possible for one new set of stereo buffers and one in progress

        auto image = ImageCache::getFromMemory(BinaryIcons::inputUsb_png, BinaryIcons::inputUsb_pngSize);
        std::shared_ptr<EffectImage> imagePtr = std::make_shared<EffectImage>(image);
        ioFileDataPtr->setImagePtr(imagePtr);

        ioFileDataPtr->effectIndexId = USB_INPUT_INDEX;
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            effectFileDataVec.push_back(ioFileDataPtr);
        }
    }

    // Add the USB Output
    {
        auto ioFileDataPtr                = std::make_shared<EffectFileData>();
        ioFileDataPtr->isInputOutput      = true;
        ioFileDataPtr->isSingleton        = true;
        ioFileDataPtr->processMidi        = false;
        ioFileDataPtr->isDataPak          = false;
        ioFileDataPtr->audioStreamType    = AudioStreamType::INT16;
        ioFileDataPtr->company            = COMPANY_NAME;
        ioFileDataPtr->effectName         = "USB OUTPUT";
        ioFileDataPtr->effectCategory     = "Output";
        ioFileDataPtr->effectCategoryEnum = EffectCategory::OUTPUT;
        ioFileDataPtr->effectDescription  = "Channel 0/1 are the LEFT/RIGHT stereo USB outputs";
        ioFileDataPtr->numControls        = 0;
        ioFileDataPtr->numInputs          = 2;
        ioFileDataPtr->numOutputs         = 0;
        ioFileDataPtr->libraryName        = "";
        ioFileDataPtr->cppClass           = "SysAudioOutputUsb";
        ioFileDataPtr->cppInstBase        = "usbOut";
        ioFileDataPtr->cpuUsage           = 0.5f;
        ioFileDataPtr->ram0Usage          = 0.0f;
        ioFileDataPtr->ram1Usage          = 0.0f;
        ioFileDataPtr->writableBuffers    = 4; // possible for one new set of stereo buffers and one in progress

        auto image = ImageCache::getFromMemory(BinaryIcons::outputUsb_png, BinaryIcons::outputUsb_pngSize);
        std::shared_ptr<EffectImage> imagePtr = std::make_shared<EffectImage>(image);
        ioFileDataPtr->setImagePtr(imagePtr);

        ioFileDataPtr->effectIndexId = USB_OUTPUT_INDEX;
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            effectFileDataVec.push_back(ioFileDataPtr);
        }
    }

    // Add the shunt
    {
        auto ioFileDataPtr                = std::make_shared<EffectFileData>();
        ioFileDataPtr->isInputOutput      = true;
        ioFileDataPtr->isSingleton        = false;
        ioFileDataPtr->processMidi        = false;
        ioFileDataPtr->isDataPak          = false;
        ioFileDataPtr->audioStreamType    = AudioStreamType::INT16;
        ioFileDataPtr->company            = COMPANY_NAME;
        ioFileDataPtr->effectName         = EFX_SHUNT_NAME;
        ioFileDataPtr->effectCategory     = "Other";
        ioFileDataPtr->effectCategoryEnum = EffectCategory::UNSPECIFIED;
        ioFileDataPtr->effectDescription  = "A shunt is used to bridge gaps or replace missing EFX.";
        ioFileDataPtr->numControls        = 0;
        ioFileDataPtr->numInputs          = 1;
        ioFileDataPtr->numOutputs         = 1;
        ioFileDataPtr->libraryName        = "";
        ioFileDataPtr->cppClass           = "AudioShunt";
        ioFileDataPtr->cppInstBase        = "audioShunt";
        ioFileDataPtr->cpuUsage           = 0.1f;
        ioFileDataPtr->ram0Usage          = 0.0f;
        ioFileDataPtr->ram1Usage          = 0.0f;
        ioFileDataPtr->writableBuffers    = 0;

        auto image = ImageCache::getFromMemory(BinaryIcons::MissingEfx_png, BinaryIcons::MissingEfx_pngSize);
        std::shared_ptr<EffectImage> imagePtr = std::make_shared<EffectImage>(image);
        ioFileDataPtr->setImagePtr(imagePtr);

        ioFileDataPtr->effectIndexId = SHUNT_INDEX;
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            effectFileDataVec.push_back(ioFileDataPtr);
        }
    }
}

void setupHandlers() {
    for (int i=0; i < NSIG; i++) {
        gPrevHandler[i] = signal(i, catch_signal);
    }
}

void restoreHanders() {
    // return the handler to the previous
    for (int i=0; i < NSIG; i++) {
        signal(i, gPrevHandler[i]);
    }
}

struct LoadEffectsCaller {
    LoadEffectsCaller() = delete;
    LoadEffectsCaller(std::vector<File> fileVecIn, std::vector<std::shared_ptr<EffectFileData>> &effectFileDataVecIn,
                      BackgroundTask* taskPtrIn, unsigned totalEfxToLoadIn, SemanticVersion coreVersionIn)
    :  fileVec(fileVecIn), effectFileDataVec(effectFileDataVecIn), taskPtr(taskPtrIn),
       totalEfxToLoad(totalEfxToLoadIn), coreVersion(coreVersionIn) {}
    virtual ~LoadEffectsCaller() = default;
    std::vector<File> fileVec;
    std::vector<std::shared_ptr<EffectFileData>> &effectFileDataVec;
    BackgroundTask* taskPtr;
    unsigned totalEfxToLoad;
    SemanticVersion coreVersion;
};

static void loadEffectsThread(void* argPtr)
{
    LoadEffectsCaller* callPtr = (LoadEffectsCaller*)argPtr;
    std::vector<File> fileVec = callPtr->fileVec;
    std::vector<std::shared_ptr<EffectFileData>>&effectFileDataVec = callPtr->effectFileDataVec;
    SemanticVersion coreVersion = callPtr->coreVersion;

    auto* defaultControlImagesPtr = DefaultControlImages::getInstance();
    defaultControlImagesPtr->loadImagesFromCache();

    for (auto efxFilePath : fileVec) {

        if (Thread::currentThreadShouldExit()) { return; }

        // use a try-catch block with a jmp buffer to convert signals off the handler stack
        // to the main program stack so we can throw and catch later to handle.
        try {

        // setup a jmp buffer to handle exceptions
        int sig;
        if ((sig = setjmp(gBuffer)) == 0) {
            // arrived locally, not through the signal handler jmp
        } else {
            throw std::runtime_error("Fatal program error while loading EFX");
        }

        // continue normal processing
        if (! efxFilePath.existsAsFile()) { return; }

        auto effectFileDataPtr   = std::make_shared<EffectFileData>();
        bool isJsonValid         = false;
        bool isLogoFound         = false;
        bool isIconFound         = false;
        bool skipThisFile        = false;
        bool isEfxFilenameValid  = false;
        bool isVersionMatch      = false;
        bool isCoreVersionCompat = false;
        bool fatalError          = false;

        effectFileDataPtr->setKnobImagePtr(std::make_shared<EffectImage>(defaultControlImagesPtr->defaultPotImage));
        effectFileDataPtr->setEncoderImagePtr(std::make_shared<EffectImage>(defaultControlImagesPtr->defaultEncoderImage));
        effectFileDataPtr->setIrSelectImagePtr(std::make_shared<EffectImage>(defaultControlImagesPtr->defaultIrSelectImage));
        effectFileDataPtr->setButtonImagePtr(std::make_shared<EffectImage>(defaultControlImagesPtr->defaultButtonOffImage),
                                                std::make_shared<EffectImage>(defaultControlImagesPtr->defaultButtonOnImage));


        // create a vector of control PNG filenames so we don't load the same one multiple times
        //std::vector<std::string> controlPngFilenamesVec;

        // Decompress the zip file
        ZipFile zipFile(efxFilePath);

        // Get the zip filename
        effectFileDataPtr->setEffectFilename(efxFilePath.getFileName().toStdString());

        std::string invalidEfxMsg = "The effect file " + efxFilePath.getFileName().toStdString() + " is invalid and will be removed. Please re-install.";

        // We have to process the .jsn file first in the Zipfile, so instead of stepping through them in order,
        // we'll create a list with the .jsn as the first entry.
        unsigned numZipEntries = (unsigned)zipFile.getNumEntries();
        std::vector<int> zipIndicesVec;
        for (unsigned i=0; i < numZipEntries; i++) {
            const ZipFile::ZipEntry* fileEntryPtr = zipFile.getEntry(i);
            if (fileEntryPtr->filename.endsWith(EFFECT_JSON_FILE_EXTENSION)) {
                zipIndicesVec.emplace(zipIndicesVec.begin(), i);  // put it at the front
            } else { zipIndicesVec.emplace_back(i); }  // add it at the back
        }

        for (unsigned j=0; j < numZipEntries; j++) {

            if (Thread::currentThreadShouldExit()) { return; }

            // for each file
            if (skipThisFile) {
                skipThisFile = false;
                break; // process no more files within this EFX file
            }

            // rather than stepping through the zip entries from the start (index j), we will
            // follow the indices from zipIndicesVec instead. The first one was setup to be
            // the JSN file. The order of the rest don't matter right now but could be ordered
            // in the future if needed.
            unsigned i = zipIndicesVec[j];
            const ZipFile::ZipEntry* fileEntryPtr = zipFile.getEntry(i);
            String filename = fileEntryPtr->filename;
            std::string msg;

            std::unique_ptr<InputStream> inputStreamPtr(zipFile.createStreamForEntry(i)); // Create an inputStream
            if (!inputStreamPtr) { continue; }

            if (filename.endsWith(EFFECT_GRAPHICS_FILE_EXTENSION)) {
                // ICON File
                auto image = PNGImageFormat().decodeImage(*inputStreamPtr);
                std::shared_ptr<EffectImage> imagePtr = std::make_shared<EffectImage>(image);

                // the company logo will be optional but the pedal icon is not
                if (filename.compareIgnoreCase(COMPANY_PNG_FILENAME) == 0) {
                    effectFileDataPtr->setCompanyLogoPtr(imagePtr);
                    isLogoFound = true;
                }
                else if (filename.compareIgnoreCase(PEDAL_PNG_FILENAME) == 0) {
                    effectFileDataPtr->setImagePtr(imagePtr);
                    isIconFound = true;
                }
                else if (filename.compareIgnoreCase(PEDAL_BASE_PNG_FILENAME) == 0) {
                    effectFileDataPtr->setBaseImagePtr(imagePtr);
                } else {
                    // other images are skipped for now
                }
            }

            if (filename.endsWith(EFFECT_BINARY_FILE_EXTENSION)) {
                // Effects File
                int64 fileLength = inputStreamPtr->getTotalLength();

                inputStreamPtr->setPosition(0);

                File tempDirectory = File::getSpecialLocation(File::SpecialLocationType::tempDirectory);
                String outputDirectoryStr = tempDirectory.getFullPathName() + String(FileUtil::fileSeparator()) + String(EFFECT_DIRECTORY_NAME);
                File outputDirectory(outputDirectoryStr);
                outputDirectory.createDirectory();

                // Rename the .dat files to .efx for a little bit of obfuscation
                // We need to get the filename without extension so we'll create a temp file
                // object for this purpose but the File class doesnt' support absolute paths, so
                // we will put it in the tempory folder. Note: this shouldn't actually create a temp file.
                File newFile = File(String(tempDirectory.getFullPathName() + File::getSeparatorString() + filename));
                String newFilename = newFile.getFileNameWithoutExtension() + "." + EFFECT_FILE_EXTENSION;
                String outputPathAndFilename = tempDirectory.getFullPathName() + File::getSeparatorString() +
                                                String(EFFECT_DIRECTORY_NAME) + File::getSeparatorString() + newFilename;

                FileOutputStream fileOutputStream(outputPathAndFilename, fileLength);
                if (fileOutputStream.failedToOpen()) {
                    displayErrorMessage("Failure on working directory!");
                    std::string msg = "loadEffectsThread(): " + std::string(EFFECT_BINARY_FILE_EXTENSION) + " filestream (length=" +
                        std::to_string(fileLength) + ") failed on " + outputPathAndFilename.toStdString();
                    errorMessage(msg);
                }
                fileOutputStream.setPosition(0); // overwrite by starting at begining of stream
                fileOutputStream.truncate();
                fileOutputStream.writeFromInputStream(*inputStreamPtr, fileLength);
            }

            if (filename.endsWith(EFFECT_JSON_FILE_EXTENSION)) {
                String jsonDataString = inputStreamPtr->readEntireStreamAsString();

                var jsonData;
                auto result = JSON::parse(jsonDataString, jsonData);

                if (!result.wasOk()) {
                    // Delete the corrupt Effects file
                    FileUtil::deleteFile(efxFilePath.getFullPathName().toStdString());

                    displayErrorMessage(invalidEfxMsg);
                    msg = "::loadEffects():ERROR, failed to parse JSON string:" + result.getErrorMessage().toStdString();
                    errorMessage(msg);

                    skipThisFile = true;
                    break; // break out of processing this JSN file within the effect file
                }

                int type = int(jsonData.getProperty("type", var()));  // Type 1 is a Devel EFX
                if (type == 1) {
                    effectFileDataPtr->isDevel = true;
                }

                effectFileDataPtr->company = jsonData.getProperty("company", var()).toString().toStdString();
                effectFileDataPtr->effectName = jsonData.getProperty("effectName", var()).toString().toStdString();
                effectFileDataPtr->effectShortName = jsonData.getProperty("effectShortName", var()).toString().toStdString();
                effectFileDataPtr->effectVersion = jsonData.getProperty("effectVersion", var()).toString().toStdString();
                effectFileDataPtr->effectCategory = jsonData.getProperty("effectCategory", var()).toString().toStdString();
                effectFileDataPtr->effectCategoryEnum = getEffectCategoryEnum(effectFileDataPtr->effectCategory);
                effectFileDataPtr->effectDescription = jsonData.getProperty("effectDescription", var()).toString().toStdString();
                effectFileDataPtr->coreVersion = jsonData.getProperty("coreVersion", var()).toString().toStdString();
                effectFileDataPtr->numControls = int(jsonData.getProperty("numControls", var()));
                effectFileDataPtr->numInputs   = int(jsonData.getProperty("numInputs", var()));
                effectFileDataPtr->numOutputs  = int(jsonData.getProperty("numOutputs", var()));
                effectFileDataPtr->isSingleton = int(jsonData.getProperty("isSingleton", var())) ? true : false;
                effectFileDataPtr->processMidi = int(jsonData.getProperty("processMidi", var())) ? true : false;
                effectFileDataPtr->isDataPak   = int(jsonData.getProperty("isDataPak", var())) ? true : false;
                if (jsonData.getProperty("audioStreamType", var()).isVoid()) {  // not specified so use INT16
                    effectFileDataPtr->audioStreamType = AudioStreamType::INT16;
                } else {
                    effectFileDataPtr->audioStreamType = static_cast<AudioStreamType>(int(jsonData.getProperty("audioStreamType", var())));
                }
                effectFileDataPtr->efxFileVersion    = jsonData.getProperty("efxFileVersion",    var()).toString().toStdString();
                effectFileDataPtr->libraryName       = jsonData.getProperty("libraryName",       var()).toString().toStdString();
                effectFileDataPtr->effectFilename    = jsonData.getProperty("effectFilename",    var()).toString().toStdString();
                effectFileDataPtr->cppClass          = jsonData.getProperty("cppClass",          var()).toString().toStdString();
                effectFileDataPtr->cppInstBase       = jsonData.getProperty("cppInstBase",       var()).toString().toStdString();
                effectFileDataPtr->constructorParams = jsonData.getProperty("constructorParams", var()).toString().toStdString();
                effectFileDataPtr->cpuUsage          = float(jsonData.getProperty("cpuUsage",  var()));
                effectFileDataPtr->ram0Usage         = float(jsonData.getProperty("ram0Usage", var()));
                effectFileDataPtr->ram1Usage         = float(jsonData.getProperty("ram1Usage", var()));
                effectFileDataPtr->writableBuffers   = int(jsonData.getProperty("writableBuffers", var()));

                // Make sure the libraryName/version and filename match
                // For backwards compatability, the version must be absent, or if present match the version in effectFileDataPtr.
                // remove the file extension first
                std::string dummyStr, filename, versionStr;
                StringUtil::splitByDelimiter(true /* first */, ".", effectFileDataPtr->getEffectFilename(), filename, dummyStr);

                // Now split the filename by '_', and get the last entry. This will return the version info if the first char is 'v'
                StringUtil::splitByDelimiter(true, "_", filename, dummyStr, versionStr);
                if (!versionStr.empty()) {
                    if (versionStr[0] == 'v') {  // version info is provided, check against effectFileDataPtr->effectVersion
                        std::string versionNumericStr = versionStr.substr(1,std::string::npos);  // remove the 'v'
                        if (versionNumericStr == effectFileDataPtr->effectVersion) { isVersionMatch = true; }
                    }
                }

                std::string nameToMatch = effectFileDataPtr->libraryName;
                if (isVersionMatch) { nameToMatch += "_" + versionStr; }
                nameToMatch += "." + EFFECT_FILE_EXTENSION;

                if (nameToMatch == effectFileDataPtr->getEffectFilename()) {
                    isEfxFilenameValid = true;
                    isVersionMatch = true;
                }

                // Post read checks
                if (effectFileDataPtr->effectVersion.empty()) { effectFileDataPtr->effectVersion = "0.0.0"; }
                if (effectFileDataPtr->coreVersion.empty())   { effectFileDataPtr->coreVersion = "0.0.0"; }

                // Get the platforms if any are specified
                auto platformsArray = jsonData.getProperty("platforms", var()).getArray();
                if (platformsArray) {
                    for (auto &platformEntry : *platformsArray) {
                        std::string platform = platformEntry.toString().toStdString();
                        if (!platform.empty() && (platform != "null")) {
                            effectFileDataPtr->platformsVec.push_back(platform);
                        }
                    }
                }

                // check for valid core version version
                SemanticVersion efxCoreVersion = SemanticVersion::strToSemVersion(effectFileDataPtr->coreVersion);
                isCoreVersionCompat = coreVersion.isCompatible(efxCoreVersion);

                auto controlArray = jsonData.getProperty("controls", var()).getArray();

                // Step through each Effect Control Entry
                int controlIndex = 0;
                for (auto &controlEntry : *controlArray) {
                    EffectControl effectControl;

                    effectControl.name            = controlEntry.getProperty("name", var()).toString().toStdString();
                    effectControl.shortName       = controlEntry.getProperty("shortName", var()).toString().toStdString();
                    effectControl.effectName      = effectFileDataPtr->effectName;
                    effectControl.effectShortName = effectFileDataPtr->effectShortName;
                    effectControl.description     = controlEntry.getProperty("description", var()).toString().toStdString();
                    effectControl.index           = controlIndex;

                    auto positionArrayVar = controlEntry.getProperty("position", var());
                    auto positionArrayPtr = positionArrayVar.getArray();
                    if (positionArrayPtr) {
                        auto positionArray = *positionArrayPtr;
                        effectControl.config.position.first    = static_cast<int>(float(positionArray[0]));
                        effectControl.config.position.second   = static_cast<int>(float(positionArray[1]));
                        effectControl.config.scalingRatio      = static_cast<float>(controlEntry.getProperty("scalingRatio", var()));
                        effectControl.config.supressValueLabel = static_cast<int>(controlEntry.getProperty("supressValueLabel", var()));
                    } else {
                        effectControl.config.position.first  = INVALID_INDEX;
                        effectControl.config.position.second = INVALID_INDEX;
                        effectControl.config.scalingRatio    = 1.0f;
                    }

                    effectControl.config.userData = static_cast<int>(controlEntry.getProperty("userData", var()));

                    auto configArrayVar = controlEntry.getProperty("config", var());
                    auto configArrayPtr = configArrayVar.getArray();

                    if (configArrayPtr) {
                        auto configArray = *configArrayPtr;
                        effectControl.config.type         = static_cast<EffectControl::Type>(int(configArray[0]));
                        effectControl.config.minValue     = static_cast<float>(float(configArray[1]));
                        effectControl.config.maxValue     = static_cast<float>(float(configArray[2]));
                        effectControl.config.defaultValue = static_cast<float>(float(configArray[3]));

                        if (configArrayPtr->size() > 4) {
                            // Optional parameters
                            effectControl.config.stepValue = static_cast<float>(float(configArray[4]));
                        }

                        switch(effectControl.config.type) {
                        // TODO: all these effects shoudl have common graphics stored in a vector of images, not make copies each time
                        case EffectControl::Type::ENCODER :
                        case EffectControl::Type::ENCODER_MONITOR :
                        {
                            std::string iconFilename = controlEntry.getProperty("iconEncoder", var()).toString().toStdString();
                            effectControl.imageFilename = iconFilename;

                            effectControl.imagePtr = std::make_shared<EffectImage>(defaultControlImagesPtr->defaultEncoderImage); // initialize
                            effectControl.imageHeight = BinaryIcons::defaultEncoder_pngHeight;
                            if (!iconFilename.empty()) {

                                //if (!isStringInArray(iconFilename, controlPngFilenamesVec)) { // not in the vector yet, so we must load it
                                    const ZipFile::ZipEntry* entryIconPtr = zipFile.getEntry(iconFilename);
                                    if (!entryIconPtr) {
                                        std::string errMsg = "loadEffects(): can't find " + iconFilename;
                                        errorMessage(errMsg);
                                        fatalError = true;
                                        skipThisFile = true;
                                        break;
                                    } else {
                                        auto encoderImage = getPngFromZip(iconFilename, zipFile);
                                        effectControl.imagePtr = std::make_shared<EffectImage>(encoderImage);
                                        effectControl.imageHeight = static_cast<int>(controlEntry.getProperty("iconEncoderHeight", var()));
                                    }
                                //}

                            }
                            break;
                        }

                        case EffectControl::Type::IR_SELECT :
                        {
                            std::string iconFilename = controlEntry.getProperty("iconIrSelect", var()).toString().toStdString();
                            effectControl.imageFilename = iconFilename;

                            effectControl.imagePtr = std::make_shared<EffectImage>(defaultControlImagesPtr->defaultIrSelectImage); // initialize
                            effectControl.imageHeight = BinaryIcons::defaultEncoder_pngHeight; // TODO make explicity IR select default knob
                            if (!iconFilename.empty()) {

                                const ZipFile::ZipEntry* entryIconPtr = zipFile.getEntry(iconFilename);
                                if (!entryIconPtr) {
                                    std::string errMsg = "loadEffects(): can't find " + iconFilename;
                                    errorMessage(errMsg);
                                    fatalError = true;
                                    skipThisFile = true;
                                    break;
                                } else {
                                    auto irSelectImage = getPngFromZip(iconFilename, zipFile);
                                    effectControl.imagePtr = std::make_shared<EffectImage>(irSelectImage);
                                    effectControl.imageHeight = static_cast<int>(controlEntry.getProperty("iconIrSelectHeight", var()));
                                }

                            }
                            break;
                        }

                        case EffectControl::Type::EXPRESSION_POT :
                        case EffectControl::Type::VALUE_MONITOR :
                        {
                            std::string iconFilename = controlEntry.getProperty("iconPot", var()).toString().toStdString();
                            effectControl.imageFilename = iconFilename;
                            int potFullRange = int(controlEntry.getProperty("potFullRange", var()));
                            if (potFullRange) { effectControl.config.fullRange = true; }

                            effectControl.imagePtr = std::make_shared<EffectImage>(defaultControlImagesPtr->defaultPotImage);
                            effectControl.imageHeight = BinaryIcons::defaultPot_pngHeight;
                            if (!iconFilename.empty()) {

                            // Check if we've already loaded this file
                            //if (!isStringInArray(iconFilename, controlPngFilenamesVec)) { // not in the vector yet, so we must load it
                                const ZipFile::ZipEntry* entryIconPtr = zipFile.getEntry(iconFilename);
                                if (!entryIconPtr) {
                                    std::string errMsg = "loadEffects(): can't find " + iconFilename;
                                    errorMessage(errMsg);
                                    fatalError = true;
                                    skipThisFile = true;
                                    break;
                                } else {
                                    auto knobImage = getPngFromZip(iconFilename, zipFile);
                                    effectControl.imagePtr = std::make_shared<EffectImage>(knobImage);
                                    effectControl.imageHeight = static_cast<int>(controlEntry.getProperty("iconPotHeight", var()));
                                }
                            }
                            break;
                        }

                        case EffectControl::Type::SWITCH_LATCHING :
                        case EffectControl::Type::SWITCH_MOMENTARY :
                        case EffectControl::Type::LED_MONITOR :
                        {
                            std::string filenameOn  = controlEntry.getProperty("iconOn",  var()).toString().toStdString();
                            std::string filenameOff = controlEntry.getProperty("iconOff", var()).toString().toStdString();
                            effectControl.imageFilename = filenameOn;
                            effectControl.image2Filename = filenameOff;

                            bool invertStatus = (effectControl.config.userData & USER_DATA_INVERT_STATUS_MASK) > USER_DATA_INVERT_STATUS_OFFSET;

                            // initialize default image based on invertStatus
                            if (invertStatus) {
                                effectControl.imagePtr  = std::make_shared<EffectImage>(defaultControlImagesPtr->defaultButtonOnImage);
                                effectControl.imageHeight = BinaryIcons::defaultButtonOn_pngHeight;
                            } else {
                                effectControl.imagePtr  = std::make_shared<EffectImage>(defaultControlImagesPtr->defaultButtonOffImage);
                                effectControl.imageHeight = BinaryIcons::defaultButtonOff_pngHeight;
                            }
                            if (!filenameOff.empty()) {

                                //if (!isStringInArray(filenameOff, controlPngFilenamesVec)) {
                                    const ZipFile::ZipEntry* entryIconOffPtr = zipFile.getEntry(filenameOff);

                                    if (!entryIconOffPtr) {
                                        std::string errMsg = "loadEffects(): can't find " + filenameOff;
                                        errorMessage(errMsg);
                                        fatalError = true;
                                        skipThisFile = true;
                                        break;
                                    }

                                    auto buttonOffImage = getPngFromZip(filenameOff, zipFile);

                                    if (!buttonOffImage.isValid()) { break; }

                                    //controlPngFilenamesVec.push_back(filenameOff);
                                    effectControl.imagePtr = std::make_shared<EffectImage>(buttonOffImage); // override if valid
                                    effectControl.imageHeight = static_cast<int>(controlEntry.getProperty("iconOffHeight", var()));
                                //}
                            }

                            // initialize defautl image based on invert status
                            if (invertStatus) {
                                effectControl.imagePtr2 = std::make_shared<EffectImage>(defaultControlImagesPtr->defaultButtonOffImage);
                                effectControl.image2Height = BinaryIcons::defaultButtonOff_pngHeight;
                            } else {
                                effectControl.imagePtr2 = std::make_shared<EffectImage>(defaultControlImagesPtr->defaultButtonOnImage);
                                effectControl.image2Height = BinaryIcons::defaultButtonOn_pngHeight;
                            }

                            if (!filenameOn.empty()) {

                                // Check if we've already loaded this file
                                //if (!isStringInArray(filenameOn, controlPngFilenamesVec)) {
                                    const ZipFile::ZipEntry* entryIconOnPtr  = zipFile.getEntry(filenameOn);

                                    if (!entryIconOnPtr) {
                                        std::string errMsg = "loadEffects(): can't find " + filenameOn;
                                        errorMessage(errMsg);
                                        fatalError = true;
                                        skipThisFile = true;
                                        break;
                                    }

                                    auto buttonOnImage  = getPngFromZip(filenameOn, zipFile);

                                    if (!buttonOnImage.isValid() ) { break; }

                                    //controlPngFilenamesVec.push_back(filenameOn);
                                    effectControl.imagePtr2 = std::make_shared<EffectImage>(buttonOnImage); // override
                                    effectControl.image2Height = static_cast<int>(controlEntry.getProperty("iconOnHeight", var()));
                                //}
                            }
                            break;
                        }

                        default : break;
                        }

                        if (effectControl.config.type == EffectControl::Type::ENCODER ||
                            effectControl.config.type == EffectControl::Type::ENCODER_MONITOR) {
                            // This is a discrete selector knob or encoder monitor, we must also read the encoder enums
                            auto enumArrayPtr = controlEntry.getProperty("enums", var()).getArray();
                            if (enumArrayPtr) { // If valid enums are present
                                for (auto &enumEntry : *enumArrayPtr) {
                                    effectControl.config.strings.push_back(enumEntry.toString().toStdString());
                                }
                            }
                        }
                    }
                    effectFileDataPtr->controlsVec.push_back(effectControl);
                    controlIndex++;
                }

                //effectSourcePtr->debugPrint();
                isJsonValid = true;
            }

            if (filename.endsWith(EFFECT_HEADER_FILE_EXTENSION)) {

                int64 fileLength = inputStreamPtr->getTotalLength();

                inputStreamPtr->setPosition(0);

                effectFileDataPtr->cppHeaderFilenameVec.push_back(filename.toStdString());

                String perEfxDirName = String(FileUtil::getFilenameWithoutExtension(effectFileDataPtr->libraryName));

                File tempDirectory = File::getSpecialLocation(File::SpecialLocationType::tempDirectory);
                String outputDirectoryStr = tempDirectory.getFullPathName() + String(FileUtil::fileSeparator()) + String(EFFECT_DIRECTORY_NAME)
                    + String(FileUtil::fileSeparator()) + perEfxDirName;
                File outputDirectory(outputDirectoryStr);
                outputDirectory.createDirectory();
                String outputPathAndFilename = outputDirectory.getFullPathName() + String(FileUtil::fileSeparator()) + filename;

                FileOutputStream fileOutputStream(outputPathAndFilename, fileLength);

                if (fileOutputStream.failedToOpen()) {
                    displayErrorMessage("Failure on working directory!");
                    std::string msg = "loadEffectsThread(): " + std::string(EFFECT_HEADER_FILE_EXTENSION) + " filestream (length=" +
                        std::to_string(fileLength) + ") failed on " + outputPathAndFilename.toStdString();
                    errorMessage(msg);
                }

                fileOutputStream.setPosition (0); // overwrite by starting at begining of file
                fileOutputStream.truncate();

                fileOutputStream.writeFromInputStream(*inputStreamPtr, fileLength);
            }

            }

            // Delete the EFX if any errors occurred
            bool mandatoryImagesFound = isIconFound && isLogoFound;
            if (fatalError || !isJsonValid || !mandatoryImagesFound || !isEfxFilenameValid || !isVersionMatch) {
                // Delete the corrupt/invalid Effects file
                FileUtil::deleteFile(efxFilePath.getFullPathName().toStdString());

                std::string msg = "::loadEffects():EFX cannot be loaded safely: ";
                if (fatalError)         { msg += " FATAL ERROR"; invalidEfxMsg += " A fatal error has occurred."; }
                else if (!isJsonValid)  { msg += " INVALID EFX CONFIG FILE"; invalidEfxMsg += " The EFX configuration is invalid."; }
                else if (!mandatoryImagesFound) { msg += " MISSING MANDATORY IMAGES"; invalidEfxMsg += " The EFX is missing mandatory images."; }
                else if (!isEfxFilenameValid) { msg += " invalid EFX filename"; invalidEfxMsg += " The filename is invalid."; }
                else if (!isVersionMatch) { msg += " invalid EFX filename"; invalidEfxMsg += " The filename version doesn't match the EFX version."; }
                errorMessage(msg);

                displayErrorMessage(invalidEfxMsg);
            } else {
                if (!isCoreVersionCompat) {
                    std::string msg = effectFileDataPtr->company + std::string(" : ") + effectFileDataPtr->effectName;
                    msg += " was built for a newer version of STRIDE Studio. It is strongly suggested you update";
                    msg += " to the latest version of the application to avoid errors.";
                    displayWarningMessage(msg);
                }
                std::lock_guard<std::mutex> lock(g_mutex);
                int effectIndex = (int)effectFileDataVec.size();

                // Before adding the EFX, check for duplicates
                bool addEfx = true;
                for (auto it=effectFileDataVec.begin(); it != effectFileDataVec.end(); ++it) {
                    std::shared_ptr<EffectFileData> checkPtr = *it;

                    if ((effectFileDataPtr->company == checkPtr->company) && (effectFileDataPtr->effectName == checkPtr->effectName)) {
                        // This EFX already had been loaded. Keep whichever version is newer
                        String outputPathAndFilename =
                                File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getFullPathName() +
                                File::getSeparatorChar() + String(APPLICATION_NAME_STR) + File::getSeparatorChar() + EFFECTS_DIR +
                                File::getSeparatorChar();

                        if (StringUtil::isNewerVersion(checkPtr->effectVersion, effectFileDataPtr->effectVersion)) {
                            // replace the old entry with this newer one
                            outputPathAndFilename += String(checkPtr->effectFilename);

                            // insert the new Efx at the old location in the vector
                            effectIndex = checkPtr->effectIndexId;  // reuse the index
                            effectFileDataVec[effectIndex] = effectFileDataPtr;  // replace the vector element
                        } else {
                            // keep the existing entry, we'll be deleting the new one
                            outputPathAndFilename += String(effectFileDataPtr->effectFilename);
                        }
                        FileUtil::deleteFile(outputPathAndFilename.toStdString());
                        addEfx = false;
                        break;
                    }
                }

                if (addEfx) {
                    effectFileDataPtr->effectIndexId = effectIndex;
                    effectFileDataVec.push_back(effectFileDataPtr);
                }
                if (callPtr->taskPtr) {
                    callPtr->taskPtr->setStatusMessage(String(effectFileDataPtr->getEffectFilename()));
                    float progress = (float)effectFileDataVec.size() / (float)callPtr->totalEfxToLoad;
                    if (progress < 0.0f) { progress = 0.0f; }
                    if (progress > 1.0f) { progress = 1.0f; }
                    callPtr->taskPtr->setProgress(progress);
                }
            }

        } catch (const std::exception& e) {
            std::string msg = "An error occured while processing EFX file " + efxFilePath.getFileName().toStdString() +
                ". It is possibly damaged  is and preventing the application from loading and will be removed. Please try " +
                "re-importing. The program will now exit.";
            std::string filename = efxFilePath.getFullPathName().toStdString();
            FileUtil::deleteFileIfExists(filename);
            displayErrorMessage(msg);
            exit(0);
        }
    }
}

void loadEffects(String filePath, std::vector<std::shared_ptr<EffectFileData>> &effectFileDataVec,
    SemanticVersion coreVersion, stride::BackgroundTask* taskPtr)
{
    // setup a temporary handler to catch segfaults that may occur during effect loading
    // and prevent the program from starting
    setupHandlers();

    effectFileDataVec.clear();
    loadInputsOutputs(effectFileDataVec);

    auto* defaultControlImagesPtr = DefaultControlImages::getInstance();
    defaultControlImagesPtr->loadImagesFromCache();

    File efxFilePath = File(filePath);
    int numEfxFiles = efxFilePath.getNumberOfChildFiles (File::findFiles, EFFECT_FILE_WILDCARD);
    int efxFileCount = 0;

    // Divde up the efx file processing into sub arrays, one array per thread.
    constexpr unsigned EFX_LOAD_MAX_THREADS = 4;
    unsigned efxPerThread = std::ceil(numEfxFiles / (float)EFX_LOAD_MAX_THREADS);
    std::vector<File> efxFileVec;
    std::vector<std::shared_ptr<ThreadTask>> threadsVec;
    std::vector<std::shared_ptr<LoadEffectsCaller>> callerArgVec;
    for (DirectoryEntry entry : RangedDirectoryIterator (efxFilePath, false, EFFECT_FILE_WILDCARD))
    {
        efxFileVec.push_back(entry.getFile());
        efxFileCount++;
        if ((efxFileCount == numEfxFiles) || (efxFileVec.size() == efxPerThread)) {
            // Launch a new thread
            // The total number of elements in effectFileDataVec will be the previously loaded inputs/outputs, plus
            // the number of EFX files
            auto callerPtr = std::make_shared<LoadEffectsCaller>(efxFileVec, effectFileDataVec, taskPtr,
                (unsigned)numEfxFiles + effectFileDataVec.size(), coreVersion);
            callerArgVec.push_back(callerPtr);

            auto ptr = std::make_shared<ThreadTask>(loadEffectsThread, "loadEffectsThread", 0, callerPtr.get());
            threadsVec.push_back(ptr);
            ptr->startThread();

            efxFileVec.clear();
        }
    }

    while(true) {
        bool threadsComplete = true;
        for (auto thread : threadsVec) {
            if (thread->isThreadRunning()) {
                if (Thread::currentThreadShouldExit()) {
                    thread->signalThreadShouldExit();
                }
                threadsComplete = false;
            }
        }
        if (threadsComplete) { break; }
    }

    // return the handler to the previous
    restoreHanders();
}

// This function searches a vector of std::strings for a match and returns true if found
bool isStringInArray(const std::string& searchString, const std::vector<std::string>& vec)
{
    bool isFound = false;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        if ( (*it).compare(searchString) == 0) {
            isFound = true;
            break;
        }
    }
    return isFound;
}

// Retrieve the specifice PNG file from the ZipFile. Returns a valid Image if valid,
// otherwise returns and invalid image. E.g. isValid() returns false;
Image getPngFromZip(const std::string& filename, ZipFile& zipFile) {
    const ZipFile::ZipEntry* fileEntryPtr = zipFile.getEntry(filename);
    //int64 originalSize = fileEntryPtr->uncompressedSize;

    std::unique_ptr<InputStream> inputStreamPtr(zipFile.createStreamForEntry(*fileEntryPtr)); // Create an inputStream
    if (!inputStreamPtr) { return Image(); }

    auto image = PNGImageFormat().decodeImage(*inputStreamPtr);

    return image;
}

std::string getEfxJson(const std::string& efxFilePath)
{
    if (!FileUtil::fileExists(efxFilePath)) { return std::string(); }

    // Decompress the zip file
    File file = File(String(efxFilePath));
    if (!file.existsAsFile()) { return std::string(); }

    ZipFile zipFile(file);

    for (unsigned i=0; i < (unsigned)zipFile.getNumEntries(); i++) {

        const ZipFile::ZipEntry* fileEntryPtr = zipFile.getEntry(i);
        String filename = fileEntryPtr->filename;

        //int64 originalSize = fileEntryPtr->uncompressedSize;
        //std::string msg = "EffectFileLoad::getEfxJson(): Processing " + filename.toStdString() + " of size " + std::to_string(originalSize);
        //noteMessage(msg);

        std::unique_ptr<InputStream> inputStreamPtr(zipFile.createStreamForEntry(i)); // Create an inputStream
        if (!inputStreamPtr) { continue; }

        if (filename.endsWith(EFFECT_JSON_FILE_EXTENSION)) {
            String jsonDataString = inputStreamPtr->readEntireStreamAsString();
            return jsonDataString.toStdString();
        }
    }

    return std::string();

}

}
