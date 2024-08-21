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

//values must align with corresponding values in enum AVMTvResolution in <avm/tv.h> in wut
enum SetResolution {
    SET_RESOLUTION_NONE    = 0,
    SET_RESOLUTION_576I    = 1,
    SET_RESOLUTION_480I    = 2,
    SET_RESOLUTION_480P    = 3,
    SET_RESOLUTION_720P    = 4,
    SET_RESOLUTION_1080I   = 6,
    SET_RESOLUTION_1080P   = 7,
    SET_RESOLUTION_576I_43 = 101,
    SET_RESOLUTION_480I_43 = 102,
    SET_RESOLUTION_480P_43 = 103,
};

enum NotificationTheme {
    NOTIFICATION_THEME_OFF,
    NOTIFICATION_THEME_DARK,
    NOTIFICATION_THEME_LIGHT,
};

#define SET_RESOLUTION_4_3_MODIFIER 100

// Wii VC
extern int32_t gAutolaunchDRCSupported;
extern int32_t gAutolaunchNoDRCSupport;
extern int32_t gDisplayOptionsOrder;
extern int32_t gSetResolution;
extern bool gUseCustomDialogs;

// Wii Mode
extern int32_t gWiiMenuSetResolution;

// WUHB Forwarder
extern int32_t gForwarderDisplayOverride;

// Other
extern int32_t gNotificationTheme;

extern bool gInWiiUMenu;
