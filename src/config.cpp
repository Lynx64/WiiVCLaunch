#include "config.h"
#include "globals.hpp"
#include "logger.h"
#include "notifications.h"
#include <string_view>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config/WUPSConfigItemStub.h>

WUPS_USE_STORAGE("WiiVCLaunch");

void boolItemCallback(ConfigItemBoolean *item, bool newValue)
{
    if (item && item->identifier) {
        if (std::string_view(USE_CUSTOM_DIALOGS_CONFIG_ID) == item->identifier) {
            gUseCustomDialogs = newValue;
            WUPSStorageAPI::Store(item->identifier, gUseCustomDialogs);
        }
    }
}

void multipleValueItemCallback(ConfigItemMultipleValues *item, uint32_t newValue)
{
    if (item && item->identifier) {
        if (std::string_view(AUTOLAUNCH_DRC_SUPPORTED_CONFIG_ID) == item->identifier) {
            gAutolaunchDRCSupported = newValue;
            WUPSStorageAPI::Store(item->identifier, gAutolaunchDRCSupported);
        } else if (std::string_view(AUTOLAUNCH_NO_DRC_SUPPORT_CONFIG_ID) == item->identifier) {
            gAutolaunchNoDRCSupport = newValue;
            WUPSStorageAPI::Store(item->identifier, gAutolaunchNoDRCSupport);
        } else if (std::string_view(DISPLAY_OPTIONS_ORDER_CONFIG_ID) == item->identifier) {
            gDisplayOptionsOrder = newValue;
            WUPSStorageAPI::Store(item->identifier, gDisplayOptionsOrder);
        } else if (std::string_view(SET_RESOLUTION_CONFIG_ID) == item->identifier) {
            gSetResolution = newValue;
            WUPSStorageAPI::Store(item->identifier, gSetResolution);
        } else if (std::string_view(WII_MENU_SET_RESOLUTION_CONFIG_ID) == item->identifier) {
            gWiiMenuSetResolution = newValue;
            WUPSStorageAPI::Store(item->identifier, gWiiMenuSetResolution);
        } else if (std::string_view(NOTIFICATION_THEME_CONFIG_ID) == item->identifier) {
            gNotificationTheme = newValue;
            WUPSStorageAPI::Store(item->identifier, gNotificationTheme);
            applyNotificationThemeSetting();
        } else if (std::string_view(FORWARDER_DISPLAY_OVERRIDE_CONFIG_ID) == item->identifier) {
            gForwarderDisplayOverride = newValue;
            WUPSStorageAPI::Store(item->identifier, gForwarderDisplayOverride);
        }
    }
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle)
{
    try {
        WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

        // Category: Wii VC settings
        auto settings = WUPSConfigCategory::Create("Wii VC settings");

        // Autolaunch (GamePad supported)
        constexpr WUPSConfigItemMultipleValues::ValuePair autolaunchDRCSupportedValues[] = {
                {DISPLAY_OPTION_CHOOSE,  "Select each time"},
                {DISPLAY_OPTION_USE_DRC, "Use \ue087 as controller"},
                {DISPLAY_OPTION_TV,      "TV Only"},
                {DISPLAY_OPTION_BOTH,    "TV and \ue087"},
                {DISPLAY_OPTION_DRC,     "\ue087 screen only"}};

        settings.add(WUPSConfigItemMultipleValues::CreateFromValue(AUTOLAUNCH_DRC_SUPPORTED_CONFIG_ID,
                                                                   "Autolaunch (\ue087 supported)",
                                                                   DEFAULT_AUTOLAUNCH_DRC_SUPPORTED_VALUE,
                                                                   gAutolaunchDRCSupported,
                                                                   autolaunchDRCSupportedValues,
                                                                   &multipleValueItemCallback));

        // Autolaunch (GamePad not supported)
        constexpr WUPSConfigItemMultipleValues::ValuePair autolaunchNoDRCSupportValues[] = {
                {DISPLAY_OPTION_CHOOSE, "Select each time"},
                {DISPLAY_OPTION_TV,     "TV Only"},
                {DISPLAY_OPTION_BOTH,   "TV and \ue087"},
                {DISPLAY_OPTION_DRC,    "\ue087 screen only"}};

        settings.add(WUPSConfigItemMultipleValues::CreateFromValue(AUTOLAUNCH_NO_DRC_SUPPORT_CONFIG_ID,
                                                                   "Autolaunch (\ue087 not supported)",
                                                                   DEFAULT_AUTOLAUNCH_NO_DRC_SUPPORT_VALUE,
                                                                   gAutolaunchNoDRCSupport,
                                                                   autolaunchNoDRCSupportValues,
                                                                   &multipleValueItemCallback));

        // Set resolution
        constexpr WUPSConfigItemMultipleValues::ValuePair setResolutionValues[] = {
                {SET_RESOLUTION_NONE,    "Same as Wii U"},
                {SET_RESOLUTION_480P,    "480p"},
                {SET_RESOLUTION_480P_43, "480p 4:3"},
                {SET_RESOLUTION_720P,    "720p"},
                {SET_RESOLUTION_480I,    "480i (non-HDMI only)"},
                {SET_RESOLUTION_480I_43, "480i 4:3 (non-HDMI only)"},
                {SET_RESOLUTION_576I,    "576i (non-HDMI, PAL only)"},
                {SET_RESOLUTION_576I_43, "576i 4:3 (non-HDMI, PAL only)"}};

        settings.add(WUPSConfigItemMultipleValues::CreateFromValue(SET_RESOLUTION_CONFIG_ID,
                                                                   "Set resolution",
                                                                   DEFAULT_SET_RESOLUTION_VALUE,
                                                                   gSetResolution,
                                                                   setResolutionValues,
                                                                   &multipleValueItemCallback));

        // Display options order
        constexpr WUPSConfigItemMultipleValues::ValuePair displayOptionsOrderValues[] = {
                {DISPLAY_OPTIONS_ORDER_DEFAULT, "Default"},
                {DISPLAY_OPTIONS_ORDER_RECENT,  "Recent"}};

        settings.add(WUPSConfigItemMultipleValues::CreateFromValue(DISPLAY_OPTIONS_ORDER_CONFIG_ID,
                                                                   "Display options order",
                                                                   DEFAULT_DISPLAY_OPTIONS_ORDER_VALUE,
                                                                   gDisplayOptionsOrder,
                                                                   displayOptionsOrderValues,
                                                                   &multipleValueItemCallback));

        // Use custom dialogs
        settings.add(WUPSConfigItemBoolean::Create(USE_CUSTOM_DIALOGS_CONFIG_ID,
                                                   "Use custom dialogs (Wii U Menu needs to be restarted)",
                                                   DEFAULT_USE_CUSTOM_DIALOGS_VALUE,
                                                   gUseCustomDialogs,
                                                   &boolItemCallback));

        // Separator
        settings.add(WUPSConfigItemStub::Create(" "));

        // Help text
        settings.add(WUPSConfigItemStub::Create("\uE06B Override Autolaunch by holding \uE000 when launching"));

        root.add(std::move(settings));

        // Category: Wii Mode settings
        auto wiiMenuSettings = WUPSConfigCategory::Create("Wii Mode settings");

        // Wii Mode set resolution
        constexpr WUPSConfigItemMultipleValues::ValuePair wiiMenuSetResolutionValues[] = {
                {SET_RESOLUTION_NONE,    "Same as Wii U"},
                {SET_RESOLUTION_480P,    "480p"},
                {SET_RESOLUTION_480P_43, "480p 4:3"},
                {SET_RESOLUTION_720P,    "720p"},
                {SET_RESOLUTION_480I,    "480i (non-HDMI only)"},
                {SET_RESOLUTION_480I_43, "480i 4:3 (non-HDMI only)"},
                {SET_RESOLUTION_576I,    "576i (non-HDMI, PAL only)"},
                {SET_RESOLUTION_576I_43, "576i 4:3 (non-HDMI, PAL only)"}};

        wiiMenuSettings.add(WUPSConfigItemMultipleValues::CreateFromValue(WII_MENU_SET_RESOLUTION_CONFIG_ID,
                                                                          "Set resolution",
                                                                          DEFAULT_WII_MENU_SET_RESOLUTION_VALUE,
                                                                          gWiiMenuSetResolution,
                                                                          wiiMenuSetResolutionValues,
                                                                          &multipleValueItemCallback));

        root.add(std::move(wiiMenuSettings));

        // Category: WUHB Forwarder settings
        auto forwarderSettings = WUPSConfigCategory::Create("WUHB Forwarder settings");

        // Override display
        constexpr WUPSConfigItemMultipleValues::ValuePair forwarderDisplayOverrideValues[] = {
                {DISPLAY_OPTION_CHOOSE, "Don't override"},
                {DISPLAY_OPTION_TV,     "TV Only"},
                {DISPLAY_OPTION_BOTH,   "TV and \uE087"},
                {DISPLAY_OPTION_DRC,    "\uE087 screen only"}};

        forwarderSettings.add(WUPSConfigItemMultipleValues::CreateFromValue(FORWARDER_DISPLAY_OVERRIDE_CONFIG_ID,
                                                                            "Override display",
                                                                            DEFAULT_FORWARDER_DISPLAY_OVERRIDE,
                                                                            gForwarderDisplayOverride,
                                                                            forwarderDisplayOverrideValues,
                                                                            &multipleValueItemCallback));

        root.add(std::move(forwarderSettings));

        // Category: Other settings
        auto otherSettings = WUPSConfigCategory::Create("Other settings");

        // Notification theme
        constexpr WUPSConfigItemMultipleValues::ValuePair notificationThemeValues[] = {
                {NOTIFICATION_THEME_OFF,   "Off"},
                {NOTIFICATION_THEME_DARK,  "Dark"},
                {NOTIFICATION_THEME_LIGHT, "Light"}};

        otherSettings.add(WUPSConfigItemMultipleValues::CreateFromValue(NOTIFICATION_THEME_CONFIG_ID,
                                                                        "Notification theme",
                                                                        DEFAULT_NOTIFICATION_THEME_VALUE,
                                                                        gNotificationTheme,
                                                                        notificationThemeValues,
                                                                        &multipleValueItemCallback));

        root.add(std::move(otherSettings));
    } catch (const std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Exception: %s", e.what());
        return WUPSCONFIG_API_CALLBACK_RESULT_ERROR;
    }
    return WUPSCONFIG_API_CALLBACK_RESULT_SUCCESS;
}

void ConfigMenuClosedCallback()
{
    // Save all changes
    WUPSStorageAPI::SaveStorage();
}

void initConfig()
{
    WUPSConfigAPIOptionsV1 configOptions = {.name = "Wii VC Launch"};
    WUPSConfigAPI_Init(configOptions, ConfigMenuOpenedCallback, ConfigMenuClosedCallback);

    WUPSStorageAPI::GetOrStoreDefault<int32_t>(AUTOLAUNCH_DRC_SUPPORTED_CONFIG_ID, gAutolaunchDRCSupported, DEFAULT_AUTOLAUNCH_DRC_SUPPORTED_VALUE);

    WUPSStorageAPI::GetOrStoreDefault<int32_t>(AUTOLAUNCH_NO_DRC_SUPPORT_CONFIG_ID, gAutolaunchNoDRCSupport, DEFAULT_AUTOLAUNCH_NO_DRC_SUPPORT_VALUE);

    WUPSStorageAPI::GetOrStoreDefault<int32_t>(DISPLAY_OPTIONS_ORDER_CONFIG_ID, gDisplayOptionsOrder, DEFAULT_DISPLAY_OPTIONS_ORDER_VALUE);

    WUPSStorageAPI::GetOrStoreDefault<int32_t>(SET_RESOLUTION_CONFIG_ID, gSetResolution, DEFAULT_SET_RESOLUTION_VALUE);

    WUPSStorageAPI::GetOrStoreDefault(USE_CUSTOM_DIALOGS_CONFIG_ID, gUseCustomDialogs, DEFAULT_USE_CUSTOM_DIALOGS_VALUE);

    if (WUPSStorageAPI::Get(WII_MENU_SET_RESOLUTION_CONFIG_ID, gWiiMenuSetResolution) == WUPS_STORAGE_ERROR_NOT_FOUND) {
        gWiiMenuSetResolution = gSetResolution;
        WUPSStorageAPI::Store(WII_MENU_SET_RESOLUTION_CONFIG_ID, gWiiMenuSetResolution);
        WUPSStorageAPI::SaveStorage();
    }

    WUPSStorageAPI::GetOrStoreDefault<int32_t>(FORWARDER_DISPLAY_OVERRIDE_CONFIG_ID, gForwarderDisplayOverride, DEFAULT_FORWARDER_DISPLAY_OVERRIDE);

    WUPSStorageAPI::GetOrStoreDefault<int32_t>(NOTIFICATION_THEME_CONFIG_ID, gNotificationTheme, DEFAULT_NOTIFICATION_THEME_VALUE);
}
