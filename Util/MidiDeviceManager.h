#pragma once

#include <JuceHeader.h>
#include "Util/CommonDefs.h"
#include "Util/Keys.h"

using namespace juce;

namespace stride {

struct Telemetry {
    float cpuUtilization;
    float ram0Usage;
    float ram1Usage;
    float temperature;
};

constexpr unsigned NUM_FUSES             = 10;
constexpr unsigned FUSE_SIZE_BYTES       = 4;

constexpr unsigned SYSEX_TYPE_SIZE_BYTES             = 1;
constexpr unsigned SYSEX_MANUFACTURER_ID_SIZE_BYTES  = 3;
constexpr unsigned PRESET_CHECKSUM_SIZE              = 4;
constexpr unsigned FUSES_SIZE_BYTES                  = NUM_FUSES * FUSE_SIZE_BYTES;
constexpr unsigned FUSE_WRITE_SIZE_BYTES             = 8;
constexpr unsigned LOCK_FUSES_SIZE_BYTES             = 4;
constexpr unsigned TELEMETRY_SIZE                    = sizeof(Telemetry);

constexpr uint8_t  BLACKADDR_AUDIO_MANUFACTURER_MIDI_ID[SYSEX_MANUFACTURER_ID_SIZE_BYTES] = {0x0, 0xA, 0xA};

constexpr unsigned COMMAND_ONLY_SYSEX_SIZE         = SYSEX_TYPE_SIZE_BYTES;

// Ping / Pong
constexpr unsigned REQUEST_PING_SYSEX_SIZE         = SYSEX_TYPE_SIZE_BYTES;
constexpr unsigned REPLY_PONG_SYSEX_SIZE           = SYSEX_TYPE_SIZE_BYTES + PRESET_CHECKSUM_SIZE + TELEMETRY_SIZE;

// UID
constexpr unsigned REQUEST_UID_SYSEX_SIZE          = SYSEX_TYPE_SIZE_BYTES;
constexpr unsigned REPLY_UID_SYSEX_SIZE            = SYSEX_TYPE_SIZE_BYTES + stride::UID_SIZE_BYTES;

// Fuses
constexpr unsigned REQUEST_FUSES_SYSEX_SIZE        = SYSEX_TYPE_SIZE_BYTES;
constexpr unsigned REPLY_FUSES_SYSEX_SIZE          = SYSEX_TYPE_SIZE_BYTES + FUSES_SIZE_BYTES;
constexpr unsigned WRITE_FUSE_SIZE_SYSEX_SIZE      = SYSEX_TYPE_SIZE_BYTES + FUSE_WRITE_SIZE_BYTES;
constexpr unsigned LOCK_FUSES_SYSEX_SIZE           = SYSEX_TYPE_SIZE_BYTES + LOCK_FUSES_SIZE_BYTES;

constexpr uint32_t PROVISIONING_PROGRAM_CHECKSUM_SIGNATURE = 0xBABABABAU;

#define GP1_LOCK_MASK_WP (0x1U << 10) // 0x400U
#define GP1_LOCK_MASK_OP (0x1U << 11) // 0x800U
#define GP2_LOCK_MASK_WP (0x1U << 12) // 0x1000U
#define GP2_LOCK_MASK_OP (0x1U << 13) // 0x2000U
#define SW_GP1_LOCK_MASK_WP_OP (0x1U << 16) // 0x010000
#define SW_GP2_LOCK_MASK_WP_OP (0x1U << 21) // 0x200000

struct MidiDeviceListEntry : public juce::ReferenceCountedObject
{
    MidiDeviceListEntry (juce::MidiDeviceInfo info) : deviceInfo (info) {}

    juce::MidiDeviceInfo deviceInfo;
    std::unique_ptr<juce::MidiInput> inDevice;
    std::unique_ptr<juce::MidiOutput> outDevice;

    using Ptr = ReferenceCountedObjectPtr<MidiDeviceListEntry>;
};

struct Fuses {
    uint32_t configLow;
    uint32_t configHigh;
    uint32_t euidhLow;
    uint32_t euidhHigh;
    uint32_t deviceKeyLow;
    uint32_t deviceKeyHigh;
    uint32_t develKeyLow;
    uint32_t develKeyHigh;
    uint32_t fusesLockLow;
    uint32_t fusesLockHigh;
};

class MidiDeviceManager : private MidiInputCallback,
                    public AsyncUpdater,
                    public ActionBroadcaster,
                    private Timer {
public:

    enum class SysExMessageType : unsigned {
        INVALID = 0,
        EFFECT_CONTROL_UPDATE = 1,
        EFFECT_STATUS_UPDATE  = 2,
        CODEC_ENABLE          = 3,
        CODEC_DISABLE         = 4,
        REQUEST_PRESET_SYNC   = 5,
        HARDWARE_PING         = 6,
        HARDWARE_PONG         = 7,

        REQUEST_INPUT_PEAK    = 8,
        REPLY_INPUT_PEAK      = 9,
        REQUEST_RESET_INPUT_PEAK = 10,
        REQUEST_ALL_INPUT_PEAK   = 11,
        REPLY_ALL_INPUT_PEAK     = 12,
        REQUEST_ALL_RESET_INPUT_PEAK = 13,

        REQUEST_OUTPUT_PEAK    = 14,
        REPLY_OUTPUT_PEAK      = 15,
        REQUEST_RESET_OUTPUT_PEAK = 16,
        REQUEST_ALL_OUTPUT_PEAK   = 17,
        REPLY_ALL_OUTPUT_PEAK     = 18,
        REQUEST_ALL_RESET_OUTPUT_PEAK = 19,

        REQUEST_UID = 20,
        REPLY_UID   = 21,

        REQUEST_FUSES = 22,
        REPLY_FUSES   = 23,

        WRITE_EUIDH       = 32,
        WRITE_DEVICE_PBKH = 33,
        WRITE_DEVEL_PBKH  = 34,
        WRITE_CONFIG      = 35,
        LOCK_FUSES        = 38,

        DEBUG_PRINT_ENABLE    = 128,
        DEBUG_PRINT_DISABLE   = 129
    };

    enum class MidiStatus : unsigned {
        DISCONNECTED = 0,
        CONNECTED,
        DESYNCED,
        DISABLED
    };

    MidiDeviceManager();
    virtual ~MidiDeviceManager() { clearSingletonInstance(); }

    void start();
    void stop();
    bool openTeensyMidi();
    bool closeTeensyMidi();
    bool openTeensyMidiOutput();
    bool openTeensyMidiInput();
    bool isTeensyMidiOpen();
    bool isMidiConnected() { return m_midiConnected; }
    bool closeTeensyMidiOutput();
    bool closeTeensyMidiInput();

    void setIsMidiDisabled(bool isMidiDisabled);
    bool getIsMidiDisabled() { return m_isMidiDisabled; }

    void updateMidiOutputDeviceList();
    void updateMidiInputDeviceList();

    void closeUnpluggedMidiOutputDevices(const Array<MidiDeviceInfo>& currentlyPluggedInDevices);
    void closeUnpluggedMidiInputDevices (const Array<MidiDeviceInfo>& currentlyPluggedInDevices);

    ReferenceCountedObjectPtr<MidiDeviceListEntry> findMidiOutDevice(MidiDeviceInfo device) const;
    ReferenceCountedObjectPtr<MidiDeviceListEntry> findMidiInDevice (MidiDeviceInfo device) const;

    bool closeMidiOutDevice(int index);
    bool closeMidiInDevice (int index);

    void debugPrintMidiDeviceList();
    void debugPrintUid();

    void addMidiStatusListener   (ActionListener *listener) { m_midiStatusBroadcaster.addActionListener(listener); forceMidiStatusBroadcast(); }
    void removeMidiStatusListener(ActionListener *listener) { m_midiStatusBroadcaster.removeActionListener(listener); }
    void forceMidiStatusBroadcast();

    using UidCallback   = std::function<void(void)>;
    using FusesCallback = std::function<void(void)>;
    void sendRequestPingSync();
    void sendRequestUid();
    void sendRequestFuses();

    void sendWriteEUIDH(uint8_t* fuseData);
    void sendWriteDevicePBKH(uint8_t* fuseData);
    void sendWriteDevelPBKH(uint8_t* fuseData);
    void sendWriteConfig(uint8_t* fuseData);
    void sendLockFuses(uint32_t fuseMask);


    void registerFusesCallback(FusesCallback callback) { m_fusesCallback = callback; }
    void setUid(stride::TeensyUid teensyUid);
    void registerUidCallback(UidCallback callback) { m_uidCallback = callback; }
    stride::TeensyUid getUid() { return m_teensyUid; }
    Fuses getFuses() { return m_fuses; }

    void setChecksumAsSignature(bool val)         { m_checksumAsSignature = val; }
    void setChecksumSignature(uint32_t signature) { m_checksumSignature = signature; }

    // use MidiDeviceManager::getInstance() to get a reference to this singleton
    JUCE_DECLARE_SINGLETON (MidiDeviceManager, true);

private:
    ReferenceCountedArray<MidiDeviceListEntry> m_midiInputs;
    ReferenceCountedArray<MidiDeviceListEntry> m_midiOutputs;

    MidiDeviceListEntry::Ptr m_teensyOutDevicePtr;
    MidiDeviceListEntry::Ptr m_teensyInDevicePtr;

    bool m_isStarted      = false;
    bool m_suppressSync   = false;
    bool m_isMidiDisabled = false;
    MidiStatus m_midiStatus          = MidiStatus::DISCONNECTED;
    bool       m_checksumAsSignature = false;
    uint32_t   m_checksumSignature   = 0;

    stride::TeensyUid m_teensyUid;
    Fuses     m_fuses;

    UidCallback m_uidCallback   = nullptr;
    UidCallback m_fusesCallback = nullptr;

    CriticalSection    m_midiMonitorLock;
    Array<MidiMessage> m_incomingMessageArray;

    // MIDI timer stuff
    ActionBroadcaster m_midiStatusBroadcaster;
    static constexpr unsigned m_MIDI_CHECK_TIMER_MS = 1000;
    std::mutex m_midiSyncLock;
    bool m_forceMidiStatusUpdate = true;
    bool m_midiWaitingForPong    = true;
    bool m_midiConnected         = false;
    void timerCallback() override;

    void processSysEx(const uint8_t* sysExData, size_t sysExDataLength);
    bool m_validateSysExManufacturerId(const uint8_t* sysExBuffer, size_t sysExDataLength) const;

    void m_sendMidiMessage(MidiMessage &message);
    void m_sendWriteFuse(SysExMessageType type, uint8_t* fuseData);

    void m_sysexMultiplexMessage(const uint8_t* byteMessage, uint8_t* nibbleMessage, size_t numBytes);
    void m_sysexDeMultiplexMessage(const uint8_t* nibbleMessage, uint8_t* byteMessage, size_t numNibbles);
    void m_sendNoPayloadCommand(SysExMessageType command);

    void m_createRequestUidMessage(uint8_t messageBuffer[2*REQUEST_UID_SYSEX_SIZE]);
    void m_createRequestFusesMessage(uint8_t messageBuffer[2*REQUEST_FUSES_SYSEX_SIZE]);
    void m_createWriteFuseMessage(uint8_t messageBuffer[2*WRITE_FUSE_SIZE_SYSEX_SIZE], SysExMessageType type, uint8_t* fuseData);
    void m_createLockFusesMessage(uint8_t messageBuffer[2*WRITE_FUSE_SIZE_SYSEX_SIZE], uint32_t fuseMask);
    void m_createRequestPingMessage(uint8_t messageBuffer[2*REQUEST_PING_SYSEX_SIZE]);
    void m_getUid(const uint8_t* sysExBuffer, size_t sysExBufferLength, stride::TeensyUid &teensyUid);
    void m_getFuses(const uint8_t* sysExBuffer, size_t sysExBufferLength, Fuses &fuses);
    void m_getPongChecksum(const uint8_t* sysExBuffer, size_t sysExBufferLength, uint32_t &pongChecksum);

    void handleIncomingMidiMessage (MidiInput *source, const MidiMessage &message) override;
    void handleAsyncUpdate() override;
};

}