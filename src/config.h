#pragma once

// Wii VC
#define DEFAULT_AUTOLAUNCH_DRC_SUPPORTED_VALUE  DISPLAY_OPTION_CHOOSE
#define DEFAULT_AUTOLAUNCH_NO_DRC_SUPPORT_VALUE DISPLAY_OPTION_CHOOSE
#define DEFAULT_DISPLAY_OPTIONS_ORDER_VALUE     DISPLAY_OPTIONS_ORDER_RECENT
#define DEFAULT_SET_RESOLUTION_VALUE            SET_RESOLUTION_NONE
#define DEFAULT_USE_CUSTOM_DIALOGS_VALUE        true

// Wii Mode
#define DEFAULT_WII_MENU_SET_RESOLUTION_VALUE   SET_RESOLUTION_NONE

// WUHB Forwarder
#define DEFAULT_FORWARDER_DISPLAY_OVERRIDE      DISPLAY_OPTION_CHOOSE

// Other
#define DEFAULT_NOTIFICATION_THEME_VALUE        NOTIFICATION_THEME_LIGHT
#define DEFAULT_PRESERVE_SYSCONF_VALUE          true
#define DEFAULT_PERMANENT_NET_CONFIG_VALUE      false

// SYSCONF
#define DEFAULT_SYSCONF_LANGUAGE_VALUE          -1
#define DEFAULT_SYSCONF_EULA_VALUE              -1

// Wii VC
#define AUTOLAUNCH_DRC_SUPPORTED_CONFIG_ID      "gAutolaunchDRCSupported"
#define AUTOLAUNCH_NO_DRC_SUPPORT_CONFIG_ID     "gAutolaunchNoDRCSupport"
#define DISPLAY_OPTIONS_ORDER_CONFIG_ID         "gDisplayOptionsOrder"
#define SET_RESOLUTION_CONFIG_ID                "gSetResolution"
#define USE_CUSTOM_DIALOGS_CONFIG_ID            "gUseCustomDialogs"

// Wii Mode
#define WII_MENU_SET_RESOLUTION_CONFIG_ID       "wiiMenuSetResolution"

// WUHB Forwarder
#define FORWARDER_DISPLAY_OVERRIDE_CONFIG_ID    "forwarderDisplayOverride"

// Other
#define NOTIFICATION_THEME_CONFIG_ID            "notificationTheme"
#define PRESERVE_SYSCONF_CONFIG_ID              "preserveSysconf"
#define PERMANENT_NET_CONFIG_CONFIG_ID          "permanentNetConfig"

// SYSCONF
#define SYSCONF_LANGUAGE_CONFIG_ID              "sysconfLanguage"
#define SYSCONF_EULA_CONFIG_ID                  "sysconfEula"

void initConfig();
