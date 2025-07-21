#include <cstring>
#include <algorithm>  // for std::remove
#include "Util/ErrorMessage.h"
#include "Util/CommonDefs.h"
#include "Util/StringUtil.h"

#include <JuceHeader.h>

using namespace juce;

namespace stride {

std::string getCoreVersionString(int major, int minor, int patch) {
    std::string coreVersionStr =
        std::to_string(major) + "." +
        std::to_string(minor) + "." +
        std::to_string(patch);
    return coreVersionStr;
}

std::string getDevicesDirectory()
{
    String dir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getFullPathName() +
        File::getSeparatorChar() + String(APPLICATION_NAME_STR) + File::getSeparatorChar() + String(DEVICES_DIR);
    return dir.toStdString();
}

std::string getDevicesFilePath()
{
    String dir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getFullPathName() +
        File::getSeparatorChar() + String(APPLICATION_NAME_STR) + File::getSeparatorChar() + String(DEVICES_DIR) +
        File::getSeparatorChar() + String(DEVICES_FILENAME);
    return dir.toStdString();
}

std::string getDevelDevicesDirectory()
{
    String dir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getFullPathName() +
        File::getSeparatorChar() + String(APPLICATION_NAME_STR) + File::getSeparatorChar() + String(DEVEL_DEVICES_DIR);
    return dir.toStdString();
}

std::string getDevelDevicesFilePath()
{
    String dir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getFullPathName() +
        File::getSeparatorChar() + String(APPLICATION_NAME_STR) + File::getSeparatorChar() + String(DEVEL_DEVICES_DIR) +
        File::getSeparatorChar() + String(DEVEL_DEVICES_FILENAME);
    return dir.toStdString();
}

std::string getProductKeysDirectory()
{
    String dir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getFullPathName() +
        File::getSeparatorChar() + String(APPLICATION_NAME_STR) + File::getSeparatorChar() + String(PRODUCT_KEYS_DIR);
    return dir.toStdString();
}


std::string getProductKeysFilePath()
{
    String dir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getFullPathName() +
        File::getSeparatorChar() + String(APPLICATION_NAME_STR) + File::getSeparatorChar() + String(PRODUCT_KEYS_DIR) +
        File::getSeparatorChar() + String(PRODUCT_KEYS_FILENAME);
    return dir.toStdString();
}

std::string getEfxDatabaseDirectory()
{
    String dir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getFullPathName() +
        File::getSeparatorChar() + String(APPLICATION_NAME_STR) + File::getSeparatorChar() + String(EFX_DATABASE_DIR);
    return dir.toStdString();
}


std::string getEfxDatabaseFilePath()
{
    String dir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getFullPathName() +
        File::getSeparatorChar() + String(APPLICATION_NAME_STR) + File::getSeparatorChar() + String(EFX_DATABASE_DIR) +
        File::getSeparatorChar() + String(EFX_DATABASE_FILENAME);
    return dir.toStdString();
}

std::string getIrDirectory()
{
    String dir = File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory).getFullPathName() +
        File::getSeparatorChar() + String(APPLICATION_NAME_STR) + File::getSeparatorChar() + String(IR_DIR);
    return dir.toStdString();
}

bool thisThreadShouldExit() {
    return Thread::currentThreadShouldExit();
}

////////////////////
// SemanticVersion
////////////////////
SemanticVersion SemanticVersion::strToSemVersion(const std::string& str)
{
    SemanticVersion version = {0, 0, 0};
    std::vector<std::string> strVec = StringUtil::splitByDelimiter(str, ".");
    if (strVec.size() >= 1) { version.major = std::stoi(strVec[0]); }
    if (strVec.size() >= 2) { version.minor = std::stoi(strVec[1]); }
    if (strVec.size() >= 3) { version.patch = std::stoi(strVec[2]); }

    return version;
}

bool SemanticVersion::isCompatible(const SemanticVersion& targetVersion)
{
    if (major != targetVersion.major) { return false; }
    if (minor < targetVersion.minor) { return false; }
    if (patch < targetVersion.patch) { return false; }
    return true;
}

}