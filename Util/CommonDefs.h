/*
 * CommonDefs.h
 *
 *  Created on: Feb. 15, 2021
 *      Author: blackaddr
 */

#ifndef UTIL_COMMONDEFS_H_
#define UTIL_COMMONDEFS_H_

#include <string>
#include <JuceHeader.h>

namespace stride {

#define UNUSED(x) (void)(x)
#define UNUSED2(x,y) (void)x; (void)y
#define UNUSED3(x,y,z) (void)x; (void)y; (void)z

static constexpr int INVALID_INSTANCE_ID = -1;
static constexpr int INVALID_INDEX       = -1;
static constexpr int SUCCESS             = 0;
static constexpr int FAILURE             = -1;
static constexpr int CANCELLED           = -2;

constexpr juce::NotificationType SEND_NOTIFY       = juce::sendNotificationAsync;
constexpr juce::NotificationType DONT_NOTIFY       = juce::dontSendNotification;
constexpr juce::NotificationType SEND_NOTIFY_SYNC  = juce::sendNotificationSync;
constexpr juce::NotificationType SEND_NOTIFY_ASYNC = juce::sendNotificationAsync;

static const std::string INDENT        = "    ";
static const std::string INDENT2       = INDENT + INDENT;
static const std::string NEWLINE       = "\n";

constexpr size_t   UID_SIZE_BYTES    = 16;
constexpr unsigned ENCODE_KEY[stride::UID_SIZE_BYTES] = {3, 5, 1, 4, 10, 7, 2, 6, 11, 13, 12, 15, 8, 14, 0, 9};

static const char EFFECTS_DIR[]            = "Effects";
static const char PRESETS_DIR[]            = "Presets";
static const char LOG_DIR[]                = "Logs";
static const char APPLICATION_NAME_STR[]   = "STRIDE";
static const char DEVICES_DIR[]            = "Devices";
static const char DEVICES_FILENAME[]       = "Devices.xml";
static const char DEVEL_DEVICES_DIR[]      = "DevelDevices";
static const char DEVEL_DEVICES_FILENAME[] = "DevelDevices.xml";
static const char PRODUCT_KEYS_DIR[]       = "ProductKeys";
static const char PRODUCT_KEYS_FILENAME[]  = "ProductKeys.xml";
static const char EFX_DATABASE_DIR[]       = "EfxDatabase";
static const char EFX_DATABASE_FILENAME[]  = "EfxDatabase.xml";
static const char IR_DIR[]                 = "IR";

constexpr char DATABASE_DIR_NAME[]      = "database";
constexpr char CUSTOMER_KEYS_DIR_NAME[] = "keys";

const std::string EFX_JSN_FILE_EXTENSION = "jsn";
const std::string EFX_FILE_EXTENSION = "efx";
const std::string EFX_WILDCARD_SEARCH = std::string("*.") + EFX_FILE_EXTENSION;

static const char EFX_INDICATOR_STR[] = "<efx>:";

static constexpr char COMPANY_NAME[] = "Blackaddr Audio";
static constexpr char EFX_SHUNT_NAME[] = "SHUNT";

// TODO: why are these here? Why not define closer to use point?
static const char DRAG_EFFECT_ENCODER_CONTROL[]  = "DRAG_EFFECT_ENCODER_CONTROL";
static const char DRAG_EFFECT_SWITCH_CONTROL[]   = "DRAG_EFFECT_SWITCH_CONTROL";
static const char DRAG_EFFECT_LED_CONTROL[]      = "DRAG_EFFECT_LED_CONTROL";

constexpr char STRIDE_RELEASE_URL[] = "http://www.blackaddr.com/downloads/stride_test/";  // KEEP FOR TESTING

#define BUILD_YEAR  (__DATE__ + 7)

// The 16-bit effect control config user data
#define USER_DATA_INVERT_STATUS_OFFSET   0
#define USER_DATA_INVERT_STATUS_MASK   0x1
#define USER_DATA_IS_MOMENTARY_OFFSET    1
#define USER_DATA_IS_MOMENTARY_MASK    0x2

std::string getCoreVersionString(int major, int minor, int patch);

std::string getEfxDatabaseDirectory();
std::string getEfxDatabaseFilePath();
std::string getIrDirectory();

bool thisThreadShouldExit(); // Checks threading library to see if termination has been requested

enum class HardwareRev : int {
    REV1 = 1,
    REV2 = 2
};

enum class ProductType : int {
    CONSUMER  = 1,
    DEVELOPER = 2
};

struct SemanticVersion {
    int major;
    int minor;
    int patch;

    bool isCompatible(const SemanticVersion& targetVersion);
    static SemanticVersion strToSemVersion(const std::string& str);
};

struct TeensyUid {
    uint8_t uid[UID_SIZE_BYTES];
    std::string getHexKey();
    std::string getAlphaKey();
    std::string getAlphaKeyPretty();
    unsigned getProductId();
    unsigned getRev();
    bool     isValid();
    static TeensyUid getUidFromAlphaKey(const std::string& key); // works for both normal and pretty formatted strings

    bool operator==(const TeensyUid& cmp)
    {
        bool isEqual = true;
        for (unsigned i=0; i < UID_SIZE_BYTES; i++) {
            if (cmp.uid[i] != uid[i]) { isEqual = false; }
        }
        return isEqual;
    }
};

TeensyUid encodeUid(const TeensyUid& teensyUid);
TeensyUid encodeUid(const TeensyUid* teensyUidPtr);
TeensyUid decodeUid(const TeensyUid* teensyUidPtr);

}

#endif /* UTIL_COMMONDEFS_H_ */
