#pragma once

#include <cstdint>

//values must align with corresponding values in enum CmptScreenType in <nn/cmpt/cmpt.h> in wut
enum DisplayOption {
    DISPLAY_OPTION_USE_DRC,
    DISPLAY_OPTION_TV,
    DISPLAY_OPTION_DRC,
    DISPLAY_OPTION_BOTH,
    DISPLAY_OPTION_CHOOSE,
};

enum DisplayOptionsOrder {
    DISPLAY_OPTIONS_ORDER_DEFAULT,
    DISPLAY_OPTIONS_ORDER_RECENT,
};

enum SetResolution {
    SET_RESOLUTION_NONE    = 0,
    SET_RESOLUTION_480P    = 3,
    SET_RESOLUTION_720P    = 4,
    SET_RESOLUTION_480P_43 = 103,
};

extern int32_t gAutolaunchDRCSupported;
extern int32_t gAutolaunchNoDRCSupport;
extern int32_t gDisplayOptionsOrder;
extern int32_t gSetResolution;
extern bool gUseCustomDialogs;

extern int32_t gWiiMenuSetResolution;

extern bool gInWiiUMenu;
