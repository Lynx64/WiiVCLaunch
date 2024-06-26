#pragma once

#define DEFAULT_AUTOLAUNCH_DRC_SUPPORTED_VALUE DISPLAY_OPTION_CHOOSE
#define DEFAULT_AUTOLAUNCH_NO_DRC_SUPPORT_VALUE DISPLAY_OPTION_CHOOSE
#define DEFAULT_DISPLAY_OPTIONS_ORDER_VALUE DISPLAY_OPTIONS_ORDER_RECENT
#define DEFAULT_SET_RESOLUTION_VALUE SET_RESOLUTION_NONE
#define DEFAULT_USE_CUSTOM_DIALOGS_VALUE true

#define DEFAULT_WII_MENU_SET_RESOLUTION_VALUE SET_RESOLUTION_NONE

#define AUTOLAUNCH_DRC_SUPPORTED_CONFIG_ID "gAutolaunchDRCSupported"
#define AUTOLAUNCH_NO_DRC_SUPPORT_CONFIG_ID "gAutolaunchNoDRCSupport"
#define DISPLAY_OPTIONS_ORDER_CONFIG_ID "gDisplayOptionsOrder"
#define SET_RESOLUTION_CONFIG_ID "gSetResolution"
#define USE_CUSTOM_DIALOGS_CONFIG_ID "gUseCustomDialogs"

#define WII_MENU_SET_RESOLUTION_CONFIG_ID "wiiMenuSetResolution"

void initConfig();
