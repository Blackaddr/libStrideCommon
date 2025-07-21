#include <iostream>
#include "Util/ErrorMessage.h"
#include "Util/CrashReport.h"

std::string CrashReport::processCrashReport(uint32_t breadcrumbs[NUM_BREADCRUMBS],
    const std::vector<std::string>& effectNamesVec, const std::vector<std::string>& presetNamesVec)
{
    std::string report = "A CRASH HAS BEEN REPORTED!\n\n";
    unsigned crashFlags = 0;
    for (unsigned i=0; i < NUM_BREADCRUMBS; i++) {
        if (breadcrumbs[i] != INVALID_BREADCRUMB) {
            // valid breadcrumb
            if ((breadcrumbs[i] & START_MASK) && !(breadcrumbs[i] & DONE_MASK)) {
                // if we see the start mask and we don't see the stop mask, this breadcrumb is the crash
                crashFlags |= (0x1 << i);
            }
        }
    }
    char text[128];
    snprintf(text, 127, "CRASH FLAGS: %08X\n", crashFlags);
    std::string crashMsg = "CRASH REPORTED!!! " + std::string(text);
    stride::errorMessage(crashMsg);

    if (crashFlags == 0) {
        report += "...but we could not recover the report data";
        return report;
    }

    if (crashFlags) {
        std::string effectName = "<unidentified>";
        std::string presetName = "<unidentified>";
        bool presetChangeCrash = crashFlags & SETPRESET_CRASH_MASK;

        if (crashFlags & AUDIO_UPDATE_CRASH_MASK) {
            unsigned data = breadcrumbs[AUDIO_EFFECT_UPDATE_ID-1] & BREADCRUMB_DATA_MASK;;
            if (data < effectNamesVec.size()) {
                effectName = effectNamesVec.at(data);
            }
        }

        if (crashFlags & SETPARAM_CRASH_MASK) {
            unsigned data = breadcrumbs[PRESET_MANAGER_SETPARAM_ID-1] & BREADCRUMB_DATA_MASK;
            if (data < effectNamesVec.size()) {
                effectName = effectNamesVec.at(data);
            }
        }

        if (crashFlags & SETPRESET_CRASH_MASK) {
            unsigned data = breadcrumbs[PRESET_MANAGER_SETPRESET_ID-1] & BREADCRUMB_DATA_MASK;
            if (data < presetNamesVec.size()) {
                presetName = presetNamesVec.at(data);
            }
        }

        // Display the erro message

        if (crashFlags & SETUP_ID_CRASH_MASK) {
            report += "The pedal crashed during setup. Please report the detailsof what happened to Blackaddr Audio.";
        }
        else if (crashFlags & AUDIO_UPDATE_CRASH_MASK) {
            report += "The pedal crashed when processing AUDIO for effect\n\n" + effectName;
            if (presetChangeCrash) { report += "\n\nduring a change to preset\n\n" + presetName; }
        }
        else if (crashFlags & SETPARAM_CRASH_MASK) {
            report += "The pedal crashed when processing a CONTROL parameter update for effect\n\n" + effectName;
            if (presetChangeCrash) { report += "\n\nduring a change to preset\n\n" + presetName; }
        }
        else if (crashFlags & SETPRESET_CRASH_MASK) {
            report += "The pedal crashed during a PRESET change to\n\n" + presetName;
        }
    }
    return report;
}