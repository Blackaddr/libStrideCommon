#pragma once

#include <string>
#include <vector>
#include <cstdint>

struct CrashReport {
    static constexpr unsigned NUM_BREADCRUMBS         = 6;

    static constexpr unsigned SETUP_ID                    = 1;
    static constexpr unsigned AUDIO_EFFECT_UPDATE_ID      = 2;
    static constexpr unsigned PRESET_MANAGER_SETPARAM_ID  = 3;
    static constexpr unsigned PRESET_MANAGER_SETPRESET_ID = 4;

    static constexpr unsigned SETUP_ID_CRASH_MASK     = 0x1U << (SETUP_ID-1);
    static constexpr unsigned AUDIO_UPDATE_CRASH_MASK = 0x1U << (AUDIO_EFFECT_UPDATE_ID-1);
    static constexpr unsigned SETPARAM_CRASH_MASK     = 0x1U << (PRESET_MANAGER_SETPARAM_ID-1);
    static constexpr unsigned SETPRESET_CRASH_MASK    = 0x1U << (PRESET_MANAGER_SETPRESET_ID-1);

    static constexpr uint32_t START_MASK           = 0x8000'0000U;
    static constexpr uint32_t DONE_MASK            = 0x4000'0000U;
    static constexpr uint32_t INVALID_BREADCRUMB   = 0xFFFF'FFFFU;
    static constexpr uint32_t BREADCRUMB_DATA_MASK = 0x00FF'FFFFU;

    static std::string processCrashReport(uint32_t breadcrumbs[NUM_BREADCRUMBS], 
        const std::vector<std::string>& effectNamesVec, const std::vector<std::string>& presetNamesVec);
};
