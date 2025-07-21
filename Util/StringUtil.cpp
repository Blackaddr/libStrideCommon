/*
 * StringUtil.cpp
 *
 *  Created on: Jan. 12, 2022
 *      Author: blackaddr
 */
#include <locale>
#include <algorithm>
#include <JuceHeader.h>  // TODO: For strToVersion(), replace std solution
#include "Util/CommonDefs.h"
#include "Util/StringUtil.h"

using namespace stride;

namespace StringUtil {

int hexCharToInt(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return FAILURE;
}

char int2HexChar(unsigned nibble, bool uppercase)
{
    if (nibble >= 0 && nibble <= 9) {
        return ('0' + nibble);
    } else if (nibble >= 0xAU && nibble <= 0xFU) {
        if (uppercase) {
            return ('A' + (nibble - 0xAU));
        } else {
            return ('a' + (nibble - 0xAU));
        }
    } else {
        return 0;  // return a string terminator
    }
}

std::string byteToHex2(uint8_t byte)
{
    std::string byteStr;
    unsigned upperNibble = (byte >> 4) & 0xFU;
    unsigned lowerNibble = (byte & 0xFU);
    byteStr.append(1,int2HexChar(upperNibble));
    byteStr.append(1,int2HexChar(lowerNibble));
    return byteStr;
}

std::string convertToAlphaNumeric(const std::string& str, unsigned maxLength)
{
	if (str.empty()) { return std::string(); }

    std::string newString;
	std::locale loc;
    unsigned strLength = 0;

	// If the first character is not alphabetic, then prepend an _ to make it legal
	if (!std::isalpha(str[0], loc)) {
	    newString.append(1, '_');
        strLength++;
	}

    for (auto & nextChar : str) {
        if ((maxLength > 0) && (strLength >= maxLength)) { break; }  // max length reached
        if (std::isalnum(nextChar, loc)) {
            newString.append(1, nextChar); // append 1 char
            strLength++;
        }
    }
    return newString;
}

std::string convertToAlphaNumericLowerCase(const std::string& str, unsigned maxLength)
{
    if (str.empty()) { return std::string(); }

    std::string lowercaseStr = convertToAlphaNumeric(str, maxLength);

    // convert string to lower case
    std::for_each(lowercaseStr.begin(), lowercaseStr.end(), [](char & c) { // use a lambda for in-line repace
        c = ::tolower(c);
    });

    return lowercaseStr;
}

std::string clipToLength(const std::string& str, unsigned maxLength)
{
    std::string strClipped = str;
    if (strClipped.size() > maxLength) {
        strClipped = strClipped.substr(0, maxLength);
    }
    return strClipped;
}

void addStringToVecConditionally(std::vector<std::string>& vec, const std::string& str)
{
    bool strFound = false;
    for (auto & stringCheck : vec) {
        if (stringCheck == str) {
            strFound = true;
        }
    }

    if (!strFound) { vec.push_back(str); }
}

int strToUint8(uint8_t* dataBuffer, size_t bufferSize, const std::string& inputDataStr)
{
    if ((inputDataStr.size() % 2) != 0) {
        return FAILURE;  // numb hex digits not divisible by 2
    }

    size_t bytesDecoded = 0;
    size_t strIdx       = 0;
    while ((bytesDecoded < bufferSize) && (strIdx < inputDataStr.size())) {
        char ch = inputDataStr[strIdx++];
        int upperNibble = hexCharToInt(ch);
        ch = inputDataStr[strIdx++];
        int lowerNibble = hexCharToInt(ch);
        int val = (upperNibble << 4) + lowerNibble;
        if (val < 0) { return  val; }
        dataBuffer[bytesDecoded++] = (uint8_t)(val & 0xFFU);
    }
    return SUCCESS;
}

int strRevToUint8(uint8_t* dataBuffer, size_t bufferSize, const std::string& dataStr)
{
    int result = strToUint8(dataBuffer, bufferSize, dataStr);
    if (result != SUCCESS) { return result; }

    // reverse the byte array
    for (unsigned i=0; i < bufferSize/2; i++) {
        uint8_t tmp = dataBuffer[i];
        dataBuffer[i] = dataBuffer[bufferSize-1-i];
        dataBuffer[bufferSize-1-i] = tmp;
    }
    return SUCCESS;
}

int uint8ToStr(uint8_t* dataBuffer, size_t bufferSize, std::string& dataStr)
{
    constexpr size_t MAX_BYTES = 1024U;
    if (bufferSize > MAX_BYTES) { return FAILURE; }

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned i = 0; i < bufferSize; ++i)
    {
        ss << std::setw(2) << static_cast<unsigned>(dataBuffer[i]);
    }
    dataStr = ss.str();
    return SUCCESS;
}

int uint8ToStrRev(uint8_t* dataBuffer, size_t bufferSize, std::string& dataStr)
{
    constexpr size_t MAX_BYTES = 1024U;
    if (bufferSize > MAX_BYTES) { return FAILURE; }

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = bufferSize-1; i >= 0; --i)
    {
        ss << std::setw(2) << static_cast<unsigned>(dataBuffer[i]);
    }
    dataStr = ss.str();
    return SUCCESS;
}

int uint8ToCodeByteArray(uint8_t* dataBuffer, size_t bufferSize, std::string& dataStr)
{
    dataStr.append("{ ");
    for (size_t idx = 0; idx < bufferSize; idx++) {
        unsigned lowerNibble = dataBuffer[idx] & 0xFU;
        unsigned upperNibble = (dataBuffer[idx] >> 4) & 0xFU;
        dataStr.append("0x");
        dataStr.append(1, int2HexChar(upperNibble));
        dataStr.append(1, int2HexChar(lowerNibble));

        if (idx < bufferSize-1) { dataStr.append(", "); }  // not the last item
    }
    dataStr.append("};");
    return SUCCESS;
}

int strToCodeByteArray(const std::string& inputDataStr, std::string& outputStr)
{
    if ((inputDataStr.size() % 2) != 0) {
        return FAILURE;  // numb hex digits not divisible by 2
    }

    outputStr.append("{ ");
    for (size_t idx = 0; idx < inputDataStr.size(); idx+=2) {
        outputStr.append("0x");
        outputStr.append(1, inputDataStr[idx]);
        outputStr.append(1, inputDataStr[idx+1]);

        if (idx < inputDataStr.size()-2) { outputStr.append(", "); }  // not the last item
    }
    outputStr.append("};");
    return SUCCESS;
}

int textToCharArray(const std::string& inputText, std::string& outputCharArray, bool appendNullTerminator)
{
    outputCharArray = "{";
    for (char byte : inputText) {
        outputCharArray += "0x";
        outputCharArray += byteToHex2(byte);
        outputCharArray += ", ";
    }

    if (appendNullTerminator) { outputCharArray += "0x0"; }
    outputCharArray += "}";

    return SUCCESS;
}

int strToVersion(const std::string& versionStr, int& major, int& minor, int& patch)
{
    juce::StringArray tokens;
    major = minor = patch = 0;
    tokens.addTokens (versionStr, ".", "\"");
    if (tokens.size() < 1) { return FAILURE; }
    if (tokens.size() >= 1) { major = tokens[0].getIntValue(); }
    if (tokens.size() >= 2) { minor = tokens[1].getIntValue(); }
    if (tokens.size() >= 3) { patch = tokens[2].getIntValue(); }
    return SUCCESS;
}

void versionToStr(int major, int minor, int patch, std::string& versionStr)
{
    versionStr = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
}

bool isNewerVersion(const std::string& currentVersion, const std::string& compareVersion)
{
    int currentMajor, currentMinor, currentPatch;
    int compareMajor, compareMinor, comparePatch;
    bool newRelease = false;

    int result = strToVersion(compareVersion, compareMajor, compareMinor, comparePatch);
    if (result != SUCCESS) { return false; }  // if the compare version is invalid, return false

    // if the compare version is valid, and the current version is empty, treat it as a new release
    if (currentVersion.empty() && !compareVersion.empty()) { newRelease = true; }
    else {
        result = strToVersion(currentVersion, currentMajor, currentMinor, currentPatch);
        if (result != SUCCESS) { newRelease = true; } // valid compareVersion, invalid currentVersion, treat as new release
        else if (compareMajor > currentMajor) { newRelease = true; }
        else if (compareMajor == currentMajor && compareMinor > currentMinor) { newRelease = true; }
        else if (compareMajor == currentMajor && compareMinor == currentMinor && comparePatch > currentPatch) { newRelease = true; }
    }
    return newRelease;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

int replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty() || str.empty()) return 0; // Avoid infinite loop if 'from' is an empty string

    size_t startPos = 0;
    int numReplacements = 0;
    while ((startPos = str.find(from, startPos)) != std::string::npos) {
        str.replace(startPos, from.length(), to);
        startPos += to.length(); // Move past the replaced part
        numReplacements++;
    }
    return numReplacements;
}

int insertEscape(std::string& str, char charToEscape)
{
    int insertCount = 0;
    int loopLimit = 1000;
    for (auto it = str.begin(); it != str.end(); ++it, loopLimit--) {
        if (loopLimit <= 0) { break; }
        if ((*it) == charToEscape) {
        //auto nextIt = it+1;
        it = str.insert(it, '\\');  // points to inserted char
        it++;  // advance forward to the original char
        insertCount++;
        }
    }
    return insertCount;
}

std::vector<std::string> splitByDelimiter(const std::string& str, const std::string& delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> tokensVec;

    if (!delimiter.empty()) {
        while ((pos_end = str.find(delimiter, pos_start)) != std::string::npos) {
            token = str.substr (pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            tokensVec.push_back (token);
        }
    }

    tokensVec.push_back (str.substr (pos_start));
    return tokensVec;
}

std::string removeWhitespace(const std::string& str)
{
    std::string cleanedStr;
    for (char c : str) {
        if (c != ' ' && c != '\t') {
            cleanedStr += c;
        }
    }
    return cleanedStr;
}

std::string toLowerCase(const std::string& str)
{
    std::string newStr = str;
    std::transform(newStr.begin(), newStr.end(), newStr.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return newStr;
}

bool splitByDelimiter(bool splitByLast, const std::string& delimiter, const std::string& inputStr, std::string& first, std::string& last)
{
    size_t pos;
    if (splitByLast) { pos = inputStr.rfind(delimiter); }  // find returns the last match
    else             { pos = inputStr.find(delimiter);  }  // rfind returns the first match
    if (pos == std::string::npos) { return false; }

    first = inputStr.substr(0, pos);
    last  = inputStr.substr(pos+1, std::string::npos);

    return true;
}

}



