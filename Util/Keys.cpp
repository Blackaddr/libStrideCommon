#include "Util/Keys.h"

using namespace juce;

namespace stride {

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

////////////////////////////////////////////////////////////////////////////////
// DEVICE KEY
////////////////////////////////////////////////////////////////////////////////
int DeviceKey::writeFile(const std::string& filepath) {
    if (!isValid) { return FAILURE; }

    auto jsonObjPtr = new DynamicObject();
    if (!jsonObjPtr) { errorMessage("DeviceKey::writeFile(): cannot allocate DynamicObject"); return FAILURE; }
    jsonObjPtr->setProperty("type", "device");
    jsonObjPtr->setProperty("UID", String(uid.getAlphaKeyPretty()));

    std::string keyStr;

    int result = StringUtil::uint8ToStr(key, PUBLIC_KEY_SIZE_BYTES, keyStr);
    if (result != SUCCESS) {
        errorMessage("DeviceKey::writeFile(): error converting uint8 array to std::str");
        return result;
    }
    jsonObjPtr->setProperty("key", String(keyStr));

    String jsonJuceStr = JSON::toString(var(jsonObjPtr));
    result = FileUtil::writeStringToFile(jsonJuceStr.toStdString(), filepath);
    if (result != SUCCESS) {
        std::string msg = "DeviceKey::writeFile(): unable to write string to file " + filepath;
        errorMessage(msg);
        return result;
    }
    return result;
}

DeviceKey DeviceKey::readFile(const std::string& filepath) {
    DeviceKey deviceKey;
    deviceKey.isValid = false;

    std::string jsonStr;
    int result = FileUtil::readFileToString(filepath, jsonStr);
    if (result) {
        displayErrorMessage("DeviceKey::readFile(): Unable to read device key file.");
        deviceKey.isValid = false;
        return deviceKey;
    }

    String jsonDataString = String(jsonStr);
    var jsonData;
    auto jsonResult = JSON::parse(jsonDataString, jsonData);

    if (!jsonResult.wasOk()) {
        displayErrorMessage("DeviceKey::readFile(): Unable to decode key device file.");
        return deviceKey;
    }

    std::string typeStr = jsonData.getProperty("type", var()).toString().toStdString();
    if (typeStr != std::string(DEVICE_TYPE_STR)) { errorMessage("DeviceKey::readFile(): incorrect key type"); return deviceKey; }

    std::string uidStr = jsonData.getProperty("UID", var()).toString().toStdString();
    if (uidStr.empty()) { errorMessage("DeviceKey::readFile(): cannot find UID property"); return deviceKey; }
    deviceKey.uid = TeensyUid::getUidFromAlphaKey(uidStr);

    std::string keyStr = jsonData.getProperty("key", var()).toString().toStdString();
    if (keyStr.empty()) { errorMessage("DeviceKey::readFile(): cannot find key property"); return deviceKey; }
    if (keyStr.empty()) {
        displayErrorMessage("DeviceKey::readFile(): Unable to decode key device file.");
        return deviceKey;
    }
    result = StringUtil::strToUint8(deviceKey.key, PUBLIC_KEY_SIZE_BYTES, keyStr);
    if (result != SUCCESS) {
        displayErrorMessage("Unable to translate device key in file.");
        return deviceKey;
    }

    deviceKey.isValid = true;
    return deviceKey;
}

////////////////////////////////////////////////////////////////////////////////
// DEVEL KEY
////////////////////////////////////////////////////////////////////////////////
int DevelKey::writeFile(const std::string& filepath) {
    if (!isValid) { return FAILURE; }

    auto jsonObjPtr = new DynamicObject();
    if (!jsonObjPtr) { errorMessage("DevelKey::writeFile(): cannot allocate DynamicObject"); return FAILURE; }
    jsonObjPtr->setProperty("type", "devel");
    jsonObjPtr->setProperty("UID", String(uid.getAlphaKeyPretty()));

    std::string keyStr;

    int result = StringUtil::uint8ToStr(pbk, PUBLIC_KEY_SIZE_BYTES, keyStr);
    if (result != SUCCESS) {
        errorMessage("DevelKey::writeFile(): error converting pbk uint8 array to std::str");
        return result;
    }
    jsonObjPtr->setProperty("pbk", String(keyStr));

    result = StringUtil::uint8ToStr(pvk, PRIVATE_KEY_SIZE_BYTES, keyStr);
    if (result != SUCCESS) {
        errorMessage("DevelKey::writeFile(): error converting pvk uint8 array to std::str");
        return result;
    }
    jsonObjPtr->setProperty("pvk", String(keyStr));

    String jsonJuceStr = JSON::toString(var(jsonObjPtr));
    result = FileUtil::writeStringToFile(jsonJuceStr.toStdString(), filepath);
    if (result != SUCCESS) {
        std::string msg = "DevelKey::writeFile(): unable to write string to file " + filepath;
        errorMessage(msg);
        return result;
    }
    return result;
}

DevelKey DevelKey::readFile(const std::string& filepath) {
    DevelKey develKey;
    develKey.isValid = false;

    std::string jsonStr;
    int result = FileUtil::readFileToString(filepath, jsonStr);
    if (result != SUCCESS || jsonStr.empty()) {
        displayErrorMessage("DevelKey::readFile(): Unable to read device key file.");
        develKey.isValid = false;
        return develKey;
    }

    String jsonDataString = String(jsonStr);
    var jsonData;
    auto jsonResult = JSON::parse(jsonDataString, jsonData);

    if (!jsonResult.wasOk()) {
        displayErrorMessage("DevelKey::readFile(): Unable to decode key device file.");
        return develKey;
    }

    std::string typeStr = jsonData.getProperty("type", var()).toString().toStdString();
    if (typeStr != std::string(DEVEL_TYPE_STR)) { errorMessage("DevelKey::readFile(): incorrect key type"); return develKey; }

    std::string uidStr = jsonData.getProperty("UID", var()).toString().toStdString();
    if (uidStr.empty()) { errorMessage("DevelKey::readFile(): cannot find UID property"); return develKey; }
    develKey.uid = TeensyUid::getUidFromAlphaKey(uidStr);

    std::string keyStr = jsonData.getProperty("pbk", var()).toString().toStdString();
    if (keyStr.empty()) { errorMessage("DevelKey DevelKey::readFile(): cannot find pbk property"); return develKey; }
    result = StringUtil::strToUint8(develKey.pbk, PUBLIC_KEY_SIZE_BYTES, keyStr);
    if (result != SUCCESS) {
        displayErrorMessage("DevelKey::readFile(): Unable to translate device key in file.");
        return develKey;
    }

    keyStr = jsonData.getProperty("pvk", var()).toString().toStdString();
    if (keyStr.empty()) { errorMessage("DevelKey DevelKey::readFile(): cannot find pvk property"); return develKey; }
    result = StringUtil::strToUint8(develKey.pvk, PRIVATE_KEY_SIZE_BYTES, keyStr);
    if (result != SUCCESS) {
        displayErrorMessage("DevelKey::readFile(): Unable to translate device key in file.");
        return develKey;
    }

    develKey.isValid = true;
    return develKey;
}

////////////////////////////////////////////////////////////////////////////////
// PRODUCT KEY
////////////////////////////////////////////////////////////////////////////////
int ProductKey::writeFileArray(const std::vector<ProductKey>& productKeyVec, const std::string& filepath, bool overwrite)
{
    auto jsonRootObjPtr = new DynamicObject();
    if (!jsonRootObjPtr) { errorMessage("ProductKey::writeFileArray(): cannot allocate DynamicObject"); return FAILURE; }

    var keyEntryArrayObj;

    if (!overwrite && FileUtil::fileExists(filepath)) {
        std::string jsonStr;
        int result = FileUtil::readFileToString(filepath, jsonStr);
        if (result) {
            displayErrorMessage("ProductKey::writeFileArray(): Unable to read device key file.");
            return FAILURE;
        }

        var jsonRootData;

        String jsonDataString = String(jsonStr);
        auto jsonResult = JSON::parse(jsonDataString, jsonRootData);

        if (!jsonResult.wasOk()) {
            displayErrorMessage("Unable to decode EFX key file.");
            return FAILURE;
        }

        auto keyArrayObjPtr = jsonRootData.getProperty("keys", var()).getArray();

        if (keyArrayObjPtr) {
            for (auto keyEntry : *keyArrayObjPtr) {
                std::string uidStr     = keyEntry.getProperty("UID",        var()).toString().toStdString();
                if (uidStr.empty()) { errorMessage("ProductKey::readFile(): cannot find UID property"); return FAILURE; }
                std::string efxCompany = keyEntry.getProperty("efxCompany", var()).toString().toStdString();
                if (efxCompany.empty()) { errorMessage("ProductKey::readFile(): cannot find efxCompany property"); return FAILURE; }
                std::string efxName    = keyEntry.getProperty("efxName",    var()).toString().toStdString();
                if (efxName.empty()) { errorMessage("ProductKey::readFile(): cannot find efxName property"); return FAILURE; }

                // Check the incoming vector for duplicates. If they exist, discard this one insert
                // the new one when processing the vector.
                for (auto keyEntryNew : productKeyVec) {
                    if ( (keyEntryNew.uid.getAlphaKeyPretty() == uidStr) &&
                         (keyEntryNew.efxCompany == efxCompany) &&
                         (keyEntryNew.efxName == efxName) )
                    {
                        // the entry already exists so we skip it
                    } else {
                        keyEntryArrayObj.append(keyEntry);
                    }
                }  // end for keys in std::vector
            }  // end for keys in json array
        }  // end if (keyArrayObjPtr)
        else {
            displayErrorMessage("Invalid EFX Key file provided");
            return FAILURE;
        }
    }

    for (auto keyEntry : productKeyVec) {
        if (!keyEntry.isValid) {
            continue;
        }

        auto jsonObjPtr = new DynamicObject();
        if (!jsonObjPtr) { errorMessage("ProductKey::writeFileArray(): cannot allocate DynamicObject"); return FAILURE; }

        jsonObjPtr->setProperty("type", "efx");
        jsonObjPtr->setProperty("UID",        String(keyEntry.uid.getAlphaKeyPretty()));
        jsonObjPtr->setProperty("efxCompany", String(keyEntry.efxCompany));
        jsonObjPtr->setProperty("efxName",    String(keyEntry.efxName));

        std::string keyStr;
        int result = StringUtil::uint8ToStr(keyEntry.key, EFX_KEY_SIZE_BYTES, keyStr);
        if (result != SUCCESS || keyStr.empty()) {
            displayErrorMessage("cannot convert key to string");
            return FAILURE;
        }
        jsonObjPtr->setProperty("key", String(keyStr));

        keyEntryArrayObj.append(var(jsonObjPtr));
    }

    jsonRootObjPtr->setProperty("keys", keyEntryArrayObj);

    String jsonJuceStr = JSON::toString(var(jsonRootObjPtr));
    int result = FileUtil::writeStringToFile(jsonJuceStr.toStdString(), filepath);
    if (result != SUCCESS) {
        std::string msg = "unable to write string to file " + filepath;
        displayErrorMessage(msg);
        return result;
    }
    return result;
}

int ProductKey::writeFile(const std::string& filepath, bool overwrite) {
    if (!isValid) {
        errorMessage("ProductKey::writeFile(): current Key is not valid");
        return FAILURE;
    }

    std::vector<ProductKey> productKeyVec;
    productKeyVec.push_back(*this);
    int result = ProductKey::writeFileArray(productKeyVec, filepath, overwrite);
    return result;
}

std::vector<ProductKey> ProductKey::readFileArray(const std::string& filepath) {
    std::vector<ProductKey> productKeyVec;

    std::string jsonStr;
    int result = FileUtil::readFileToString(filepath, jsonStr);
    if (result) {
        displayErrorMessage("ProductKey::readFile(): Unable to read device key file.");
        return productKeyVec;
    }

    String jsonDataString = String(jsonStr);
    var jsonData;
    auto jsonResult = JSON::parse(jsonDataString, jsonData);

    if (!jsonResult.wasOk()) {
        displayErrorMessage("ProductKey::readFile(): Unable to decode key device file.");
        return productKeyVec;
    }

    auto keyArrayObjPtr = jsonData.getProperty("keys", var()).getArray();
    if (!keyArrayObjPtr) {
        displayErrorMessage("ProductKey::readFile(): Unable to decode key device file.");
        return productKeyVec;
    }

    auto keyArray = *keyArrayObjPtr;

    for (auto keyEntry : keyArray) {
        ProductKey productKey;
        std::string typeStr = keyEntry.getProperty("type", var()).toString().toStdString();
        if (typeStr != std::string(EFX_TYPE_STR)) { displayErrorMessage("incorrect key type"); continue; }
        std::string uidStr    = keyEntry.getProperty("UID", var()).toString().toStdString();
        if (uidStr.empty()) { displayErrorMessage("cannot find UID property"); continue; }
        productKey.uid        = TeensyUid::getUidFromAlphaKey(uidStr);
        productKey.efxCompany = keyEntry.getProperty("efxCompany", var()).toString().toStdString();
        if (productKey.efxCompany.empty()) { displayErrorMessage("cannot find efxCompany property"); continue; }
        productKey.efxName    = keyEntry.getProperty("efxName", var()).toString().toStdString();
        if (productKey.efxName.empty()) { displayErrorMessage("cannot find efxName property"); continue; }

        std::string keyStr = keyEntry.getProperty("key", var()).toString().toStdString();
        if (keyStr.empty()) { displayErrorMessage("cannot find key property"); continue; }
        result = StringUtil::strToUint8(productKey.key, EFX_KEY_SIZE_BYTES, keyStr);

        if (result != SUCCESS) { continue; }
        productKey.isValid = true;
        productKeyVec.push_back(productKey);
    }

    return productKeyVec;
}

ProductKey ProductKey::readFile(const std::string& filepath) {
    std::vector<ProductKey> productKeyVec = ProductKey::readFileArray(filepath);
    if (productKeyVec.empty()) { return ProductKey(); }
    return productKeyVec[0];
}

}
