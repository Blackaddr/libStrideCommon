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

std::string TeensyUid::getHexKey()
{
    std::string key;

    for (size_t i=0; i < UID_SIZE_BYTES; i++) {
        char hex[4];
        if (i == UID_SIZE_BYTES-1) {
            snprintf(hex, 4, "%02X", uid[i]); // last byte don't print the :
        } else {
            snprintf(hex, 4, "%02X:", uid[i]);
        }
        key += std::string(hex);
    }
    return key;
}

// Product ID is second last byte
unsigned TeensyUid::getProductId()
{
    return uid[UID_SIZE_BYTES-1];
}

// Rev ID is second last byte
unsigned TeensyUid::getRev()
{
    return uid[UID_SIZE_BYTES-2];
}

bool TeensyUid::isValid()
{
    if (getProductId() == 0) { return false; }
    if (getRev() == 0)       { return false; }

    bool uidIsZero = true;
    for (size_t i=0; i < UID_SIZE_BYTES; i++) {
        if (uid[i] != 0) { uidIsZero = false; }
    }
    if (uidIsZero) { return false; }

    bool uidIsOnes = true;
    for (size_t i=0; i < UID_SIZE_BYTES; i++) {
        if (uid[i] != 0xFFU) { uidIsOnes = false; }
    }

    return !uidIsOnes;
}

char getKeyChar(uint8_t value)
{
    // 5-bit values are mapped from 0-31 into a char
    // A-Z maps to 0 to 23, with 'I' and 'O' removed
    // 2-9 maps to 24 to 31 with '1' and '0' removed
    // A	0
    // B	1
    // C	2
    // D	3
    // E	4
    // F	5
    // G	6
    // H	7
    // J	8
    // J	9
    // L	10
    // M	11
    // N	12
    // P	13
    // Q	14
    // R	15
    // S	16
    // T	17
    // U	18
    // V	19
    // W	20
    // X	21
    // Y	22
    // Z	23
    // 2	24
    // 3	25
    // 4	26
    // 5	27
    // 6	28
    // 7	29
    // 8	30
    // 9    31

    if (value < 8) {
        return ('A' + value);  // value between 0 and 7 inclusive
    } else if (value < 13) {
        return ('J' + value - 8);  // value between 8 and 12 inclusive
    } else if (value < 24) {
        return ('P' + value - 13);  // value between 13 and 23 inclusive
    } else if (value < 32) {
        return ('2' + value - 24);  // value between 24 and 31 inclusive
    } else {
        return ('0');
    }
}

uint8_t getHexFromChar(char value)
{
    if (value >= 'A' && value <= 'H') {
        return value - 'A';  // return value between 0 and 7 inclusive
    } else if (value >= 'J' && value <= 'N') {
        return value - 'J' + 8;  // return value between 8 and 12 inclusive
    } else if (value >= 'P' && value <= 'Z') {
        return value - 'P' + 13;  // return value between 13 and 23 inclusive
    } else if (value >= '2' && value <= '9') {
        return value - '2' + 24;  // return value between 24 and 31 inclusive
    } else {
        return 0;
    }
}

std::string TeensyUid::getAlphaKey()
{
    std::string key;

    // convert the 16-byte value to alphanumeric encoding where
    size_t bitCount = 0;
    while (bitCount < (8*UID_SIZE_BYTES)) {
        size_t byteIndex = (bitCount / 8);
        size_t bitIndex  = bitCount % 8;
        uint16_t byte;
        if (byteIndex+1 < UID_SIZE_BYTES) {
            byte = (uid[byteIndex+1] << 8) + uid[byteIndex];
        } else {
            byte = uid[byteIndex];
        }
        byte = (byte >> bitIndex) & 0x1F;
        char alphaChar = getKeyChar(byte);
        key += std::string(1, alphaChar);

        //printf("bitCount %d: byteIndex:%d bitIndex:%d byte:%02X char:%c key:%s\n", bitCount, byteIndex, bitIndex, byte, alphaChar, key.c_str());
        bitCount += 5;
    }
    return key;
}

std::string TeensyUid::getAlphaKeyPretty()
{
    std::string str = getAlphaKey();
    if (str.empty()) { return str; }
    std::string prettyStr;

    size_t charCount = 0;
    //for (char const &letter : str) {
    for (auto it = str.begin(); it != str.end(); ++it) {
        char const &letter = *it;
        prettyStr += std::string(1, letter);
        charCount++;
        if ( (it != str.begin()) && (it != str.end()-1) && (charCount % 5 == 0) ) {
            prettyStr += '-';
        }
    }
    return prettyStr;
}

TeensyUid TeensyUid::getUidFromAlphaKey(const std::string& keyIn)
{
    TeensyUid uid;
    std::memset(uid.uid, 0, UID_SIZE_BYTES);
    if (keyIn.empty()) {
        errorMessage("TeensyUid::getUidFromAlphaKey(): UID in is empty");
        return uid;
    }
    std::string key = keyIn; // make mutable

    // If the key contains '-' because it's pretty version, remove the '-'
    key.erase(std::remove(key.begin(), key.end(), '-'), key.end());

    size_t bitCount = 0;

    for (char const &letter : key) {
        size_t byteIndex = (bitCount / 8);
        size_t bitIndex  = bitCount % 8;
        uint16_t byte5 = getHexFromChar(letter);

        uint16_t mask = static_cast<uint16_t>(0x1F) << bitIndex;
        uint16_t data = byte5 << bitIndex;

        if (bitIndex + 5 >= 8) {  // this 5-bits goes across a byte boundary
            uid.uid[byteIndex] &= static_cast<uint8_t>(~(mask & 0xFF)); // clear the bits first
            uid.uid[byteIndex] |= (data & 0xFF);
            mask = mask >> 8;
            data = data >> 8;

            if ((byteIndex+1) < UID_SIZE_BYTES) {
                uid.uid[byteIndex+1] &= static_cast<uint8_t>(~(mask & 0xFF)); // clear the bits first
                uid.uid[byteIndex+1] |= (data & 0xFF);
            }

        } else {  // completely contained within a byte boundary
            uid.uid[byteIndex] &= static_cast<uint8_t>(~(mask & 0xFF)); // clear the bits first
            uid.uid[byteIndex] |= (data & 0xFF);
        }
        bitCount += 5;
    }

    return uid;
}

TeensyUid encodeUid(const TeensyUid& teensyUid) {
    TeensyUid encodedUid;
    for (unsigned i=0; i < UID_SIZE_BYTES; i++) {
        encodedUid.uid[i] = (teensyUid.uid[i] + ENCODE_KEY[i]) & 0xFFU;
    }

    // shuffle the bytes
    for (unsigned i=0; i < UID_SIZE_BYTES; i++) {
        uint8_t tmp = encodedUid.uid[ENCODE_KEY[i]];
        encodedUid.uid[ENCODE_KEY[i]] = encodedUid.uid[i];
        encodedUid.uid[i] = tmp;
    }

    return encodedUid;
}

TeensyUid encodeUid(const TeensyUid* teensyUidPtr)
{
    return encodeUid(*teensyUidPtr);
}

TeensyUid decodeUid(const TeensyUid* teensyUidPtr)
{
    TeensyUid decodedUid = *teensyUidPtr;

    // de-shuffle the bytes
    for (int i=UID_SIZE_BYTES-1; i >= 0; i--) {
        uint8_t tmp = decodedUid.uid[i];
        decodedUid.uid[i] = decodedUid.uid[ENCODE_KEY[i]];
        decodedUid.uid[ENCODE_KEY[i]] = tmp;
    }

    for (unsigned i=0; i < UID_SIZE_BYTES; i++) {
        decodedUid.uid[i] = (decodedUid.uid[i] - ENCODE_KEY[i]) & 0xFFU;
    }


    return decodedUid;
}

}