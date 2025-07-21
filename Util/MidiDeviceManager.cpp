#include "Util/ErrorMessage.h"
#include "Util/ErrorMessageWindow.h"
#include "Util/CommonDefs.h"
#include "MidiDeviceManager.h"

namespace stride {

JUCE_IMPLEMENT_SINGLETON(MidiDeviceManager)

MidiDeviceManager::MidiDeviceManager()
{
    startTimer(m_MIDI_CHECK_TIMER_MS);
    memset((void*)&m_teensyUid.uid[0], 0xff, UID_SIZE_BYTES);
}

void MidiDeviceManager::start()
{
    m_isStarted = true;
    //REGISTER_CATCH_SIGNAL_HANDLERS();
}

void MidiDeviceManager::stop()
{
    m_isStarted = false;
    //RESTORE_DEFAULT_SIGNAL_HANDLERS();
}

void MidiDeviceManager::updateMidiOutputDeviceList()
{
    // Update output devices
    auto availableOutputDevices = MidiOutput::getAvailableDevices();
    {
        ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = m_midiOutputs;
        closeUnpluggedMidiOutputDevices(availableOutputDevices);
        ReferenceCountedArray<MidiDeviceListEntry> newDeviceList;

        // add all currently plugged-in devices to the device list
        for (auto& newDevice : availableOutputDevices)
        {
            MidiDeviceListEntry::Ptr entry = findMidiOutDevice (newDevice);

            if (entry == nullptr)
                entry = new MidiDeviceListEntry (newDevice);

            newDeviceList.add (entry);
        }

        // actually update the device list
        midiDevices = newDeviceList;
    }
}

void MidiDeviceManager::updateMidiInputDeviceList()
{
    // Update input devices
    auto availableInputDevices = MidiInput::getAvailableDevices();
    {
        ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = m_midiInputs;
        closeUnpluggedMidiOutputDevices(availableInputDevices);
        ReferenceCountedArray<MidiDeviceListEntry> newDeviceList;

        // add all currently plugged-in devices to the device list
        for (auto& newDevice : availableInputDevices)
        {
            MidiDeviceListEntry::Ptr entry = findMidiOutDevice (newDevice);

            if (entry == nullptr)
                entry = new MidiDeviceListEntry (newDevice);

            newDeviceList.add (entry);
        }

        // actually update the device list
        midiDevices = newDeviceList;
    }
}

void MidiDeviceManager::closeUnpluggedMidiOutputDevices (const Array<MidiDeviceInfo>& currentlyPluggedInDevices)
{
    ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = m_midiOutputs;

    for (auto i = midiDevices.size(); --i >= 0;)
    {
        auto& d = *midiDevices[i];

        if (! currentlyPluggedInDevices.contains (d.deviceInfo))
        {
            if (d.outDevice.get() != nullptr) {
                closeMidiOutDevice (i);
            }

            midiDevices.remove (i);
        }
    }
}

void MidiDeviceManager::closeUnpluggedMidiInputDevices (const Array<MidiDeviceInfo>& currentlyPluggedInDevices)
{
    ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = m_midiInputs;

    for (auto i = midiDevices.size(); --i >= 0;)
    {
        auto& d = *midiDevices[i];

        if (! currentlyPluggedInDevices.contains (d.deviceInfo))
        {
            if (d.inDevice.get() != nullptr) {
                closeMidiInDevice (i);
            }

            midiDevices.remove (i);
        }
    }
}

ReferenceCountedObjectPtr<MidiDeviceListEntry> MidiDeviceManager::findMidiOutDevice (MidiDeviceInfo device) const
{
    const ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = m_midiOutputs;

    for (auto& d : midiDevices)
        if (d->deviceInfo == device)
            return d;

    return nullptr;
}

ReferenceCountedObjectPtr<MidiDeviceListEntry> MidiDeviceManager::findMidiInDevice (MidiDeviceInfo device) const
{
    const ReferenceCountedArray<MidiDeviceListEntry>& midiDevices = m_midiInputs;

    for (auto& d : midiDevices)
        if (d->deviceInfo == device)
            return d;

    return nullptr;
}

bool MidiDeviceManager::closeMidiOutDevice (int index)
{
    //jassert (m_midiOutputs[index]->outDevice != nullptr);
    if (m_midiOutputs[index]) {
        m_midiOutputs[index]->outDevice = nullptr;
        return true;
    }
    return false;
}

bool MidiDeviceManager::closeMidiInDevice (int index)
{
    if (m_midiInputs[index]) {
        if (m_midiInputs[index]->inDevice) {
            m_midiInputs[index]->inDevice->stop();
            m_midiInputs[index]->inDevice = nullptr;
            return true;
        }
    }
    return false;
}

void MidiDeviceManager::debugPrintMidiDeviceList()
{
    std::string msg = "*** MIDI OUTPUT DEVICE LIST ***";
    noteMessage(msg);
    for (auto deviceEntry : m_midiOutputs) {
        MidiDeviceListEntry& midiOutputEntry = *deviceEntry;
        msg = "ident: " + midiOutputEntry.deviceInfo.identifier.toStdString() + "  name: " + midiOutputEntry.deviceInfo.name.toStdString();
        noteMessage(msg);
    }

    msg = "*** MIDI INPUT DEVICE LIST ***";
    noteMessage(msg);
    for (auto deviceEntry : m_midiInputs) {
        MidiDeviceListEntry& midiInputEntry = *deviceEntry;
        std::string msg = "ident: " + midiInputEntry.deviceInfo.identifier.toStdString() + "  name: " + midiInputEntry.deviceInfo.name.toStdString();
        noteMessage(msg);
    }
}

void MidiDeviceManager::debugPrintUid()
{
    std::string msg = "MidiDeviceManager::debugPrintUid(): TeensyUid: ";
    for (unsigned i=0; i < UID_SIZE_BYTES; i++) {
        char hex[3];
        snprintf(hex, 3, "%02X", m_teensyUid.uid[i]);
        msg += std::string(hex) + " ";
    }
    noteMessage(msg);
}

bool MidiDeviceManager::openTeensyMidiOutput()
{
    updateMidiOutputDeviceList();
    //debugPrintMidiDeviceList();

    for (int i=0; i < m_midiOutputs.size(); i++) {
        if (!m_midiOutputs[i]) { continue; }
        MidiDeviceListEntry& midiOutputEntry = *m_midiOutputs[i];

        if (midiOutputEntry.deviceInfo.name.contains("Teensy")) {
            // open the device

            if ( m_midiOutputs[i]->outDevice) {
                // already open
                return true;
            }
            jassert (m_midiOutputs[i]->outDevice.get() == nullptr);
            m_midiOutputs[i]->outDevice = MidiOutput::openDevice(m_midiOutputs[i]->deviceInfo.identifier);

            if (m_midiOutputs[i]->outDevice.get() == nullptr)
            {
                return false;
            } else {
                m_teensyOutDevicePtr = m_midiOutputs[i];
                return true;
            }
        }
    }

    return false;
}

bool MidiDeviceManager::openTeensyMidiInput()
{
    updateMidiInputDeviceList();
    //debugPrintMidiDeviceList();

    for (int i=0; i < m_midiInputs.size(); i++) {
        if (!m_midiInputs[i]) { continue; }
        MidiDeviceListEntry& midiInputEntry = *m_midiInputs[i];

        if (midiInputEntry.deviceInfo.name.contains("Teensy")) {
            // open the device
            if ( m_midiInputs[i]->inDevice) {
                // already open
                return true;
            }
            jassert (m_midiInputs[i]->inDevice.get() == nullptr);
            m_midiInputs[i]->inDevice = MidiInput::openDevice(m_midiInputs[i]->deviceInfo.identifier, this);

            if (m_midiInputs[i]->inDevice.get() == nullptr)
            {
                return false;
            } else {
                m_teensyInDevicePtr = m_midiInputs[i];
                m_teensyInDevicePtr->inDevice->start();
                return true;
            }
        }
    }

    return false;
}

bool MidiDeviceManager::openTeensyMidi()
{
    bool isOutputOpen = openTeensyMidiOutput();
    bool isInputOpen  = openTeensyMidiInput();

    return isOutputOpen && isInputOpen;
}

bool MidiDeviceManager::closeTeensyMidi()
{
    bool closedOutput = closeTeensyMidiOutput();
    bool closedInput  = closeTeensyMidiInput();
    return (closedOutput || closedInput);
}

bool MidiDeviceManager::isTeensyMidiOpen()
{
    bool isOutputOpen = false;
    if (m_teensyOutDevicePtr) {
        if (m_teensyOutDevicePtr->outDevice) {
            isOutputOpen = true;
        }
    }

    bool isInputOpen = false;
    if (m_teensyInDevicePtr) {
        if (m_teensyInDevicePtr->inDevice) {
            isInputOpen = true;
        }
    }
    return isOutputOpen && isInputOpen;
}


bool MidiDeviceManager::closeTeensyMidiOutput()
{
    updateMidiOutputDeviceList();
    bool isClosed = false;

    for (int i=0; i < m_midiOutputs.size(); i++) {
        MidiDeviceListEntry& midiOutputEntry = *m_midiOutputs[i];

        if (midiOutputEntry.deviceInfo.name.contains("Teensy")) {
            // close the device
            if (m_midiOutputs[i]->outDevice) {
                m_midiOutputs[i]->outDevice = nullptr;
                m_teensyOutDevicePtr = nullptr;
                isClosed = true;
            }
        }
    }
    return isClosed;
}

bool MidiDeviceManager::closeTeensyMidiInput()
{
    updateMidiInputDeviceList();
    bool isClosed = false;

    for (int i=0; i < m_midiInputs.size(); i++) {
        MidiDeviceListEntry& midiInputEntry = *m_midiInputs[i];

        if (midiInputEntry.deviceInfo.name.contains("Teensy")) {
            // close the device
            if (m_midiInputs[i]->inDevice) {
                m_midiInputs[i]->inDevice->stop();
                m_midiInputs[i]->inDevice = nullptr;
                m_teensyInDevicePtr = nullptr;
                isClosed = true;
            }
        }
    }
    return isClosed;
}

void MidiDeviceManager::m_sendNoPayloadCommand(SysExMessageType command)
{
    constexpr unsigned ACTUAL_PAYLOAD_SIZE = 2*COMMAND_ONLY_SYSEX_SIZE;
    uint8_t messageBuffer[ACTUAL_PAYLOAD_SIZE];

    uint8_t type = static_cast<uint8_t>(command);

    // Create a temp message buffer to contruct a normal payload, then multiplex it into the real messageBuffer as nibbles.
    uint8_t tempMessageBuffer[COMMAND_ONLY_SYSEX_SIZE];

    constexpr unsigned TYPE_START_IDX  = 0;
    constexpr unsigned TYPE_SIZE_BYTES = sizeof(type);

    if (messageBuffer) {
            std::memcpy(tempMessageBuffer + TYPE_START_IDX,   (void*)&type,   TYPE_SIZE_BYTES);
    }

    // Multiplex the tempMessageBuffer into messageBuffer
    int j=0;
    for (unsigned i=0; i < COMMAND_ONLY_SYSEX_SIZE; i++) {
        messageBuffer[j]   = tempMessageBuffer[i] & 0xF; // grab the lower nibble
        messageBuffer[j+1] = (tempMessageBuffer[i] & 0xF0) >> 4; // grab the upper nibble
        j = j+2;
    }


    uint8_t rawSysEx[SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE];

    std::memcpy(rawSysEx, BLACKADDR_AUDIO_MANUFACTURER_MIDI_ID, SYSEX_MANUFACTURER_ID_SIZE_BYTES);
    std::memcpy(rawSysEx+SYSEX_MANUFACTURER_ID_SIZE_BYTES, messageBuffer, ACTUAL_PAYLOAD_SIZE);

    MidiMessage sysExMsg = MidiMessage::createSysExMessage(rawSysEx,SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE);
    m_sendMidiMessage(sysExMsg);
}

void MidiDeviceManager::sendRequestPingSync()
{
    constexpr unsigned ACTUAL_PAYLOAD_SIZE = 2*REQUEST_PING_SYSEX_SIZE;
    uint8_t messagePayload[ACTUAL_PAYLOAD_SIZE];

    m_createRequestPingMessage(messagePayload);

    uint8_t rawSysEx[SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE];

    std::memcpy(rawSysEx, BLACKADDR_AUDIO_MANUFACTURER_MIDI_ID, SYSEX_MANUFACTURER_ID_SIZE_BYTES);
    std::memcpy(rawSysEx+SYSEX_MANUFACTURER_ID_SIZE_BYTES, messagePayload, ACTUAL_PAYLOAD_SIZE);

    MidiMessage sysExMsg = MidiMessage::createSysExMessage(rawSysEx,SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE);
    m_sendMidiMessage(sysExMsg);
}

void MidiDeviceManager::sendRequestUid()
{
    constexpr unsigned ACTUAL_PAYLOAD_SIZE = 2*REQUEST_UID_SYSEX_SIZE;
    uint8_t messagePayload[ACTUAL_PAYLOAD_SIZE];

    m_createRequestUidMessage(messagePayload);

    uint8_t rawSysEx[SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE];

    std::memcpy(rawSysEx, BLACKADDR_AUDIO_MANUFACTURER_MIDI_ID, SYSEX_MANUFACTURER_ID_SIZE_BYTES);
    std::memcpy(rawSysEx+SYSEX_MANUFACTURER_ID_SIZE_BYTES, messagePayload, ACTUAL_PAYLOAD_SIZE);

    MidiMessage sysExMsg = MidiMessage::createSysExMessage(rawSysEx,SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE);
    m_sendMidiMessage(sysExMsg);
}

void MidiDeviceManager::sendRequestFuses()
{
    constexpr unsigned ACTUAL_PAYLOAD_SIZE = 2*REQUEST_FUSES_SYSEX_SIZE;
    uint8_t messagePayload[ACTUAL_PAYLOAD_SIZE];

    m_createRequestFusesMessage(messagePayload);

    uint8_t rawSysEx[SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE];

    std::memcpy(rawSysEx, BLACKADDR_AUDIO_MANUFACTURER_MIDI_ID, SYSEX_MANUFACTURER_ID_SIZE_BYTES);
    std::memcpy(rawSysEx+SYSEX_MANUFACTURER_ID_SIZE_BYTES, messagePayload, ACTUAL_PAYLOAD_SIZE);

    MidiMessage sysExMsg = MidiMessage::createSysExMessage(rawSysEx,SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE);
    m_sendMidiMessage(sysExMsg);
}

void MidiDeviceManager::sendWriteEUIDH(uint8_t* fuseData)
{
    m_sendWriteFuse(SysExMessageType::WRITE_EUIDH, fuseData);
}

void MidiDeviceManager::sendWriteDevicePBKH(uint8_t* fuseData)
{
    m_sendWriteFuse(SysExMessageType::WRITE_DEVICE_PBKH, fuseData);
}

void MidiDeviceManager::sendWriteDevelPBKH(uint8_t* fuseData)
{
    m_sendWriteFuse(SysExMessageType::WRITE_DEVEL_PBKH, fuseData);
}

void MidiDeviceManager::sendWriteConfig(uint8_t* fuseData)
{
    m_sendWriteFuse(SysExMessageType::WRITE_CONFIG, fuseData);
}

void MidiDeviceManager::sendLockFuses(uint32_t fuseMask)
{
    constexpr unsigned ACTUAL_PAYLOAD_SIZE = 2*LOCK_FUSES_SYSEX_SIZE;
    uint8_t messagePayload[ACTUAL_PAYLOAD_SIZE];

    m_createLockFusesMessage(messagePayload, fuseMask);

    uint8_t rawSysEx[SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE];

    std::memcpy(rawSysEx, BLACKADDR_AUDIO_MANUFACTURER_MIDI_ID, SYSEX_MANUFACTURER_ID_SIZE_BYTES);
    std::memcpy(rawSysEx+SYSEX_MANUFACTURER_ID_SIZE_BYTES, messagePayload, ACTUAL_PAYLOAD_SIZE);

    MidiMessage sysExMsg = MidiMessage::createSysExMessage(rawSysEx,SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE);

    m_sendMidiMessage(sysExMsg);
}

void MidiDeviceManager::m_sendWriteFuse(SysExMessageType type, uint8_t* fuseData)
{
    constexpr unsigned ACTUAL_PAYLOAD_SIZE = 2*WRITE_FUSE_SIZE_SYSEX_SIZE;
    uint8_t messagePayload[ACTUAL_PAYLOAD_SIZE];

    m_createWriteFuseMessage(messagePayload, type, fuseData);

    uint8_t rawSysEx[SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE];

    std::memcpy(rawSysEx, BLACKADDR_AUDIO_MANUFACTURER_MIDI_ID, SYSEX_MANUFACTURER_ID_SIZE_BYTES);
    std::memcpy(rawSysEx+SYSEX_MANUFACTURER_ID_SIZE_BYTES, messagePayload, ACTUAL_PAYLOAD_SIZE);

    MidiMessage sysExMsg = MidiMessage::createSysExMessage(rawSysEx,SYSEX_MANUFACTURER_ID_SIZE_BYTES + ACTUAL_PAYLOAD_SIZE);

    m_sendMidiMessage(sysExMsg);
}

void MidiDeviceManager::m_sendMidiMessage(MidiMessage &message)
{
    if (!m_teensyOutDevicePtr) {
        //openTeensyMidiOutput(); // try to open
        if (!m_teensyOutDevicePtr) { return; }
    } // TODO ERROR MESSAGE POPUP
    if (!m_teensyOutDevicePtr->outDevice) { return; } // TODO ERROR MESSAGE POPUP

    if (!m_isMidiDisabled) {
        m_teensyOutDevicePtr->outDevice->sendMessageNow(message);
    }
}

void MidiDeviceManager::m_createRequestPingMessage(uint8_t messageBuffer[2*REQUEST_PING_SYSEX_SIZE])
{
    uint8_t type = static_cast<uint8_t>(SysExMessageType::HARDWARE_PING);

    // Create a temp message buffer to contruct a normal payload, then multiplex it into the real messageBuffer as nibbles.
    uint8_t tempMessageBuffer[REQUEST_PING_SYSEX_SIZE];

    constexpr unsigned TYPE_START_IDX  = 0;
    constexpr unsigned TYPE_SIZE_BYTES = sizeof(type);

    if (messageBuffer) {
            std::memcpy(tempMessageBuffer + TYPE_START_IDX,   (void*)&type,   TYPE_SIZE_BYTES);
    }

    // Multiplex the tempMessageBuffer into messageBuffer
    int j=0;
    for (unsigned i=0; i < REQUEST_PING_SYSEX_SIZE; i++) {
        messageBuffer[j]   = tempMessageBuffer[i] & 0xF; // grab the lower nibble
        messageBuffer[j+1] = (tempMessageBuffer[i] & 0xF0) >> 4; // grab the upper nibble
        j = j+2;
    }
}

// UID
void MidiDeviceManager::m_createRequestUidMessage(uint8_t messageBuffer[2*REQUEST_UID_SYSEX_SIZE])
{
    uint8_t type = static_cast<uint8_t>(SysExMessageType::REQUEST_UID);

    // Create a temp message buffer to contruct a normal payload, then multiplex it into the real messageBuffer as nibbles.
    uint8_t tempMessageBuffer[REQUEST_UID_SYSEX_SIZE];

    constexpr unsigned TYPE_START_IDX  = 0;
    constexpr unsigned TYPE_SIZE_BYTES = sizeof(type);

    if (messageBuffer) {
            std::memcpy(tempMessageBuffer + TYPE_START_IDX,   (void*)&type,   TYPE_SIZE_BYTES);
    }

    // Multiplex the tempMessageBuffer into messageBuffer
    int j=0;
    for (unsigned i=0; i < REQUEST_UID_SYSEX_SIZE; i++) {
        messageBuffer[j]   = tempMessageBuffer[i] & 0xF; // grab the lower nibble
        messageBuffer[j+1] = (tempMessageBuffer[i] & 0xF0) >> 4; // grab the upper nibble
        j = j+2;
    }
}

// FUSES
void MidiDeviceManager::m_createRequestFusesMessage(uint8_t messageBuffer[2*REQUEST_FUSES_SYSEX_SIZE])
{
    uint8_t type = static_cast<uint8_t>(SysExMessageType::REQUEST_FUSES);

    // Create a temp message buffer to contruct a normal payload, then multiplex it into the real messageBuffer as nibbles.
    uint8_t tempMessageBuffer[REQUEST_FUSES_SYSEX_SIZE];

    constexpr unsigned TYPE_START_IDX  = 0;
    constexpr unsigned TYPE_SIZE_BYTES = sizeof(type);

    if (messageBuffer) {
            std::memcpy(tempMessageBuffer + TYPE_START_IDX,   (void*)&type,   TYPE_SIZE_BYTES);
    }

    // Multiplex the tempMessageBuffer into messageBuffer
    int j=0;
    for (unsigned i=0; i < REQUEST_UID_SYSEX_SIZE; i++) {
        messageBuffer[j]   = tempMessageBuffer[i] & 0xF; // grab the lower nibble
        messageBuffer[j+1] = (tempMessageBuffer[i] & 0xF0) >> 4; // grab the upper nibble
        j = j+2;
    }
}

void MidiDeviceManager::m_createWriteFuseMessage(uint8_t messageBuffer[2*WRITE_FUSE_SIZE_SYSEX_SIZE], SysExMessageType type, uint8_t* fuseData)
{
    uint8_t typeByte = static_cast<uint8_t>(type);

    // Create a temp message buffer to contruct a normal payload, then multiplex it into the real messageBuffer as nibbles.
    uint8_t tempMessageBuffer[WRITE_FUSE_SIZE_SYSEX_SIZE];

    constexpr unsigned TYPE_START_IDX       = 0;
    constexpr unsigned TYPE_SIZE_BYTES      = sizeof(typeByte);
    constexpr unsigned FUSE_DATA_START_IDX  = TYPE_START_IDX + TYPE_SIZE_BYTES;
    constexpr unsigned FUSE_DATA_BYTES      = FUSE_WRITE_SIZE_BYTES;

    if (messageBuffer) {
        std::memcpy(tempMessageBuffer + TYPE_START_IDX,      (void*)&typeByte, TYPE_SIZE_BYTES);
        std::memcpy(tempMessageBuffer + FUSE_DATA_START_IDX, (void*)fuseData,  FUSE_DATA_BYTES);
    }

    // Multiplex the tempMessageBuffer into messageBuffer
    int j=0;
    for (unsigned i=0; i < WRITE_FUSE_SIZE_SYSEX_SIZE; i++) {
        messageBuffer[j]   = tempMessageBuffer[i] & 0xF; // grab the lower nibble
        messageBuffer[j+1] = (tempMessageBuffer[i] & 0xF0) >> 4; // grab the upper nibble
        j = j+2;
    }
}

void MidiDeviceManager::m_createLockFusesMessage(uint8_t messageBuffer[2*LOCK_FUSES_SYSEX_SIZE], uint32_t fuseMask)
{
    uint8_t type = static_cast<uint8_t>(SysExMessageType::LOCK_FUSES);

    // Create a temp message buffer to contruct a normal payload, then multiplex it into the real messageBuffer as nibbles.
    uint8_t tempMessageBuffer[LOCK_FUSES_SYSEX_SIZE];

    constexpr unsigned TYPE_START_IDX  = 0;
    constexpr unsigned TYPE_SIZE_BYTES = sizeof(type);
    constexpr unsigned FUSE_MASK_START_IDX  = TYPE_START_IDX + TYPE_SIZE_BYTES;
    constexpr unsigned FUSE_MASK_BYTES      = LOCK_FUSES_SIZE_BYTES;

    if (messageBuffer) {
            std::memcpy(tempMessageBuffer + TYPE_START_IDX,      (void*)&type,     TYPE_SIZE_BYTES);
            std::memcpy(tempMessageBuffer + FUSE_MASK_START_IDX, (void*)&fuseMask, FUSE_MASK_BYTES);
    }

    // Multiplex the tempMessageBuffer into messageBuffer
    int j=0;
    for (unsigned i=0; i < LOCK_FUSES_SYSEX_SIZE; i++) {
        messageBuffer[j]   = tempMessageBuffer[i] & 0xF; // grab the lower nibble
        messageBuffer[j+1] = (tempMessageBuffer[i] & 0xF0) >> 4; // grab the upper nibble
        j = j+2;
    }
}

// MISC MIDI PROCESING
void MidiDeviceManager::m_sysexMultiplexMessage(const uint8_t* byteMessage, uint8_t* nibbleMessage, size_t numBytes)
{
    // Multiplex the byteMessage into nibbleMessage
    if (!byteMessage || !nibbleMessage) { return; }
    unsigned j=0;
    for (size_t i=0; i < numBytes; i++) {
        nibbleMessage[j]   = byteMessage[i] & 0xF; // grab the lower nibble
        nibbleMessage[j+1] = (byteMessage[i] & 0xF0) >> 4; // grab the upper nibble
        j = j+2;
    }
}

void MidiDeviceManager::m_sysexDeMultiplexMessage(const uint8_t* nibbleMessage, uint8_t* byteMessage, size_t numNibbles)
{
    // DeMultiplex the nibbleMessage into byteMessage
    if (!nibbleMessage || !byteMessage) { return; }
    size_t numBytes = numNibbles/2;
    unsigned j=0;
    for (size_t i=0; i < numBytes; i++) {
        byteMessage[i] = (nibbleMessage[j] & 0xF) + ((nibbleMessage[j+1] & 0xF) << 4);
        j = j+2;
    }
}

void MidiDeviceManager::handleIncomingMidiMessage (MidiInput *source, const MidiMessage &message) {
    // This is called on the MIDI thread
    const ScopedLock lock (m_midiMonitorLock);
    m_incomingMessageArray.add(message);
    triggerAsyncUpdate();
}

void MidiDeviceManager::handleAsyncUpdate()
{
    // This is called on the message loop
    Array<MidiMessage> messages;
    {
        const ScopedLock sl (m_midiMonitorLock);
        messages.swapWith (m_incomingMessageArray);
    }

    if (m_isMidiDisabled || !m_isStarted) { return; }

    //REGISTER_CATCH_SIGNAL_HANDLERS();

    //SIGNAL_ENTRY_CHECK("MidiDeviceManager::handleAsyncUpdate(): ERROR: a fatal error has occured while processing an incoming MIDI messaage");

    try {

    for (auto& midiMessage : messages) {

        if (midiMessage.isSysEx()) {
            const uint8_t* rawSysExPtr = reinterpret_cast<const uint8_t*>(midiMessage.getSysExData());
            int      sysExSize   = midiMessage.getSysExDataSize();
            processSysEx(rawSysExPtr, sysExSize);
        }
    }
    } catch (const std::exception& c) {
        //RESTORE_DEFAULT_SIGNAL_HANDLERS();
        displayErrorMessage("A program crash was caught while processing an incoming MIDI message. Please disconnect \
                             the USB cable. Click OK, then rebuild, reconnecdt the pedal, push the programming button \
                             on the pedal, then reprogram.");
        return;
    }
    //RESTORE_DEFAULT_SIGNAL_HANDLERS();
}

void MidiDeviceManager::processSysEx(const uint8_t* sysExData, size_t sysExDataLength)
{
    // Skip the SYSEX start 0xF0 byte

    if (!m_validateSysExManufacturerId(sysExData, sysExDataLength) ) { // Skip the SYSEX START 0xF0 byte
        errorMessage("MidiDeviceManager::processSysEx(): manufacturer ID doesn't match\n");
        return;
    }
    sysExData       += SYSEX_MANUFACTURER_ID_SIZE_BYTES; // advance past the SYSEX manufacturer ID
    sysExDataLength -= SYSEX_MANUFACTURER_ID_SIZE_BYTES; // update remaining size

    SysExMessageType type = static_cast<SysExMessageType>((sysExData[0] & 0xF) + ((sysExData[1] & 0xF) << 4));
    sysExData       += 2*SYSEX_TYPE_SIZE_BYTES;
    sysExDataLength -= 2*SYSEX_TYPE_SIZE_BYTES;

    switch(static_cast<SysExMessageType>(type)) {

    case SysExMessageType::HARDWARE_PONG :
    {
        if (m_checksumAsSignature) {
            uint32_t signature = 0;
            m_getPongChecksum(sysExData, sysExDataLength, signature);
            if (signature == m_checksumSignature) {
                m_suppressSync = false;
            } else {
                m_suppressSync = true;
            }
        } else {
            m_suppressSync = false;
        }

        {
            std::lock_guard<std::mutex> lock(m_midiSyncLock);
            m_midiWaitingForPong = false;
        }
    }
    break;

    case SysExMessageType::REPLY_FUSES :
    {
        m_getFuses(sysExData, sysExDataLength, m_fuses);
        if (m_fusesCallback) { m_fusesCallback(); }
    }
    break;

    case SysExMessageType::REPLY_UID :
    {
        TeensyUid teensyUid;
        m_getUid(sysExData, sysExDataLength, teensyUid);
        setUid(teensyUid);
        if (m_uidCallback) { m_uidCallback(); }
    }
    break;

    default:
        break;
    }
}

void MidiDeviceManager::setUid(TeensyUid teensyUid) {
    m_teensyUid = teensyUid;
}

bool MidiDeviceManager::m_validateSysExManufacturerId(const uint8_t* sysExBuffer, size_t sysExDataLength) const
{
    if (!sysExBuffer || (sysExDataLength < SYSEX_MANUFACTURER_ID_SIZE_BYTES)) { return false; }

    for (unsigned i=0; i < SYSEX_MANUFACTURER_ID_SIZE_BYTES; i++) {
        if (sysExBuffer[i] != BLACKADDR_AUDIO_MANUFACTURER_MIDI_ID[i]) {
            return false;
        }
    }
    return true;
}

// UID
void MidiDeviceManager::m_getUid(const uint8_t* sysExBuffer, size_t sysExBufferLength, TeensyUid &teensyUid)
{
    constexpr unsigned EXPECTED_SIZE = 2*(REPLY_UID_SYSEX_SIZE - SYSEX_TYPE_SIZE_BYTES);
    if (!sysExBuffer || (sysExBufferLength != EXPECTED_SIZE)) {
        std::string errorMsg = "MidiDeviceManager::m_getUid(): SYSEX control message wrong size, expected " +
                               std::to_string(EXPECTED_SIZE) + ", received " + std::to_string(sysExBufferLength);
        errorMessage(errorMsg);
        return;
    }

    constexpr unsigned UID_START_IDX  = 0;
    constexpr unsigned UID_SIZE_BYTES = sizeof(teensyUid.uid);

    uint8_t messageBuffer[EXPECTED_SIZE];

    // de-multiplex the datastream
    unsigned j=0;
    for (unsigned i=0; i < EXPECTED_SIZE; i++) {
        messageBuffer[i] = (sysExBuffer[j] & 0xF) + ((sysExBuffer[j+1] & 0xF) << 4);
        j = j + 2;
    }

    std::memcpy((void*)&teensyUid.uid[0], messageBuffer+UID_START_IDX,  UID_SIZE_BYTES);
}

// FUSES
void MidiDeviceManager::m_getFuses(const uint8_t* sysExBuffer, size_t sysExBufferLength, Fuses& fuses)
{
    constexpr unsigned EXPECTED_SIZE = 2*(REPLY_FUSES_SYSEX_SIZE - SYSEX_TYPE_SIZE_BYTES);
    if (!sysExBuffer || (sysExBufferLength != EXPECTED_SIZE)) {
        std::string errorMsg = "MidiDeviceManager::m_getFuses(): SYSEX control message wrong size, expected " +
                               std::to_string(EXPECTED_SIZE) + ", received " + std::to_string(sysExBufferLength);
        errorMessage(errorMsg);
        return;
    }

    constexpr unsigned FUSES_START_IDX  = 0;
    constexpr unsigned FUSES_SIZE_BYTES = NUM_FUSES * sizeof(uint32_t);

    uint8_t messageBuffer[EXPECTED_SIZE];

    // de-multiplex the datastream
    unsigned j=0;
    for (unsigned i=0; i < EXPECTED_SIZE; i++) {
        messageBuffer[i] = (sysExBuffer[j] & 0xF) + ((sysExBuffer[j+1] & 0xF) << 4);
        j = j + 2;
    }

    // configLow is the first field so copy to it's address
    std::memcpy((void*)&fuses.configLow, messageBuffer+FUSES_START_IDX,  FUSES_SIZE_BYTES);
}

void MidiDeviceManager::m_getPongChecksum(const uint8_t* sysExBuffer, size_t sysExBufferLength, uint32_t &pongChecksum)
{
    uint32_t  checksum = 0;

    constexpr unsigned EXPECTED_SIZE = 2*(REPLY_PONG_SYSEX_SIZE - SYSEX_TYPE_SIZE_BYTES);
    if (!sysExBuffer || (sysExBufferLength != EXPECTED_SIZE)) {
        std::string errorMsg = "MidiManager::m_getPongChecksum(): SYSEX control message wrong size, expected " +
                               std::to_string(EXPECTED_SIZE) + ", received " + std::to_string(sysExBufferLength);
        errorMessage(errorMsg);
        return;
    }

    constexpr unsigned CHECKSUM_START_IDX  = 0;
    constexpr unsigned CHECKSUM_SIZE_BYTES = sizeof(checksum);

    uint8_t messageBuffer[EXPECTED_SIZE];

    // de-multiplex the datastream
    unsigned j=0;
    for (unsigned i=0; i < EXPECTED_SIZE; i++) {
        messageBuffer[i] = (sysExBuffer[j] & 0xF) + ((sysExBuffer[j+1] & 0xF) << 4);
        j = j + 2;
    }

    std::memcpy((void*)&checksum, messageBuffer+CHECKSUM_START_IDX,  CHECKSUM_SIZE_BYTES);

    pongChecksum = checksum;
}

void MidiDeviceManager::setIsMidiDisabled(bool isMidiDisabled) {
    m_isMidiDisabled = isMidiDisabled;
    std::string msg = m_isMidiDisabled ? "DISABLED" : "DISCONNECTED";
    m_midiStatusBroadcaster.sendActionMessage(msg);
    if (m_isMidiDisabled) {
        m_midiStatus = MidiStatus::DISABLED;
    } else {
        m_midiStatus = MidiStatus::DISCONNECTED;
        m_midiWaitingForPong = false;
    }
}

void MidiDeviceManager::timerCallback()
{
    static bool resendConnectedMessage = false;
    if (m_isMidiDisabled) {
        resendConnectedMessage = true;
        return;
    }

    if (m_suppressSync && m_midiStatus != MidiStatus::DESYNCED) {
        m_midiStatus = MidiStatus::DESYNCED;
        std::string msg = "DESYNCED";
        m_midiStatusBroadcaster.sendActionMessage(msg);
    } else if (!m_suppressSync && m_midiStatus == MidiStatus::DESYNCED) {
        std::string msg = "CONNECTED";
        m_midiStatusBroadcaster.sendActionMessage(msg);
    }

    if (resendConnectedMessage) {
        updateMidiOutputDeviceList();
        updateMidiInputDeviceList();

        if (isTeensyMidiOpen()) {
            std::string msg = "CONNECTED";
            m_midiStatusBroadcaster.sendActionMessage(msg);
        }
        resendConnectedMessage = false;
    }

    bool midiConnectedPrevous = m_midiConnected;
    {
        std::lock_guard<std::mutex> lock(m_midiSyncLock);

        m_midiConnected = m_midiWaitingForPong ? false : true;

        if (m_midiWaitingForPong) { // Midi did not respond to last request try re-opening

            // Windows does not let you close and reopen a device in the same call. Another thread in JUCE
            // needs to do some clean up, so we only attempt to open if closing() returns false (already closed previously).
            // This way we close on one timer callback and open on the next
            bool outputIsClosed = closeTeensyMidi();
            if (!outputIsClosed) {
                bool success = openTeensyMidi();
                (void)success;
            }
        }
        m_midiWaitingForPong = true;

        if ((midiConnectedPrevous != m_midiConnected) || m_forceMidiStatusUpdate) {
            m_forceMidiStatusUpdate = false;
            std::string msg = m_midiConnected ? "CONNECTED" : "DISCONNECTED";
            if (m_midiConnected) {
                m_midiStatus = MidiStatus::CONNECTED;
            } else {
                m_midiStatus = MidiStatus::DISCONNECTED;
            }
            m_midiStatusBroadcaster.sendActionMessage(msg);
        }
    }
    sendRequestPingSync();
    sendRequestUid();
}

void MidiDeviceManager::forceMidiStatusBroadcast()
{
    {
        std::lock_guard<std::mutex> lock(m_midiSyncLock);
        m_forceMidiStatusUpdate = true;
    }
}

}