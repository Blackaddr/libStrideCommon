/*
 * StringUtil.h
 *
 *  Created on: Jan. 12, 2022
 *      Author: blackaddr
 */

#ifndef SOURCE_LIBSTRIDECOMMON_UTIL_STRINGUTIL_H_
#define SOURCE_LIBSTRIDECOMMON_UTIL_STRINGUTIL_H_

#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <iomanip>

/// Convert integer value `val` to text in hexadecimal format.
/// The minimum width is padded with leading zeros; if not
/// specified, this `width` is derived from the type of the
/// argument. Function suitable from char to long long.
/// Pointers, floating point values, etc. are not supported;
/// passing them will result in an (intentional!) compiler error.
template <typename T>
inline std::string intToHex(T val, size_t width=sizeof(T)*2)
{
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(width) << std::hex << (val|0);
    return ss.str();
}

/// TODO move to stride namespace
namespace StringUtil {

std::string convertToAlphaNumeric(const std::string& str, unsigned maxLength = 0);

std::string convertToAlphaNumericLowerCase(const std::string& str, unsigned maxLength = 0);

std::string clipToLength(const std::string& str, unsigned maxLength);

void addStringToVecConditionally(std::vector<std::string>& vec, const std::string& str);

int hexCharToInt(char ch);  // converts a single hex char to an int
char int2HexChar(unsigned nibble, bool uppercase = false);  // converts an unsigned nibble to a hex char

// convert a string of hex digits to binary uint8_t
int strToUint8(uint8_t* dataBuffer, size_t bufferSize, const std::string& dataStr);

// convert a string of hex digits to binary uint8_t in reverse order
int strRevToUint8(uint8_t* dataBuffer, size_t bufferSize, const std::string& dataStr);

// convert a uint8_t byte array to hex string
int uint8ToStr(uint8_t* dataBuffer, size_t bufferSize, std::string& dataStr);

// convert a uint8_t byte array to a hex string in reverse order
int uint8ToStrRev(uint8_t* dataBuffer, size_t bufferSize, std::string& dataStr);
std::string byteToHex2(uint8_t byte);  // converts a byte to a 2-digit hex string

/// this function will return a byte array definition suitable for defining C arrays.
int uint8ToCodeByteArray(uint8_t* dataBuffer, size_t bufferSize, std::string& dataStr);

// This function will take in a hex array and output a code byte array suitable for defining arrays in C.
int strToCodeByteArray(const std::string& inputDataStr, std::string& outputStr);

// Convert regular text to a string array of chars, suitable for defining arrays in C.
// It will start and end wtih curly braces.
int textToCharArray(const std::string& inputText, std::string& outputCharArray, bool appendNullTerminator = false);

int  strToVersion(const std::string& versionStr, int& major, int& minor, int& patch);
void versionToStr(int major, int minor, int patch, std::string& versionStr);
bool isNewerVersion(const std::string& currentVersion, const std::string& compareVersion);

// This function will find the string 'from' in 'str' and replace it with 'to'
bool replace(std::string& str, const std::string& from, const std::string& to);

/// This function will replace all occurences of the string 'from' with 'to' in 'str'.
/// @returns the number of replacements performed
int replaceAll(std::string& str, const std::string& from, const std::string& to);

// This function will insert an escape before any instances of the specified character. It returns the
// number of insertions
int insertEscape(std::string& str, char charToEscape);

std::string removeWhitespace(const std::string& str);

std::string toLowerCase(const std::string& str);

// split a string in two by the provided delimiter. When splitByLast is true, returns the
// string after the delimiter, otherwise returns the first.
bool splitByDelimiter(bool splitByLast, const std::string& delimiter, const std::string& inputStr, std::string& first, std::string& last);

// Split a string in to a vector of string using a delimiter string
std::vector<std::string> splitByDelimiter(const std::string& str, const std::string& delimiter);

}
#endif /* SOURCE_LIBSTRIDECOMMON_UTIL_STRINGUTIL_H_ */
