#include "config.h"
#include "globals.hpp"
#include "lang.h"
#include "logger.h"
#include "notifications.h"

#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <wups/config/WUPSConfigItemStub.h>

#include <string_view>

WUPS_USE_STORAGE("WiiVCLaunch");

static Language sLanguageSetting = Language::System;

void boolItemCallback(ConfigItemBoolean *item, bool newValue)
{
    if (!item || !item->identifier) {
        return;
    }
    if (std::string_view(USE_CUSTOM_DIALOGS_CONFIG_ID) == item->identifier) {
        gUseCustomDialogs = newValue;
        WUPSStorageAPI::Store(item->identifier, gUseCustomDialogs);
    } else if (std::string_view(PRESERVE_SYSCONF_CONFIG_ID) == item->identifier) {
        gPreserveSysconf = newValue;
        WUPSStorageAPI::Store(item->identifier, gPreserveSysconf);
    } else if (std::string_view(PERMANENT_NET_CONFIG_CONFIG_ID) == item->identifier) {
        gPermanentNetConfig = newValue;
        WUPSStorageAPI::Store(item->identifier, gPermanentNetConfig);
    }
}

void multipleValueItemCallback(ConfigItemMultipleValues *item, uint32_t newValue)
{
    if (!item || !item->identifier) {
        return;
    }
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

void languageChangedCallback(ConfigItemMultipleValues *item, uint32_t newValue)
{
    sLanguageSetting = static_cast<Language>(newValue);
    setLanguage(sLanguageSetting);
    WUPSStorageAPI::Store(LANGUAGE_CONFIG_ID, sLanguageSetting);
}

WUPSConfigAPICallbackStatus ConfigMenuOpenedCallback(WUPSConfigCategoryHandle rootHandle)
{
    const TranslatedStrings strings = getTranslatedStrings();

    try {
        WUPSConfigCategory root = WUPSConfigCategory(rootHandle);

        // Category: Wii VC settings
        auto settings = WUPSConfigCategory::Create(strings.wii_vc_settings);

        // Autolaunch (GamePad supported)
        const WUPSConfigItemMultipleValues::ValuePair autolaunchDRCSupportedValues[] = {
                {DISPLAY_OPTION_CHOOSE,  strings.select_each_time},
                {DISPLAY_OPTION_USE_DRC, strings.use_drc_as_controller},
                {DISPLAY_OPTION_TV,      strings.tv_only},
                {DISPLAY_OPTION_BOTH,    strings.tv_and_drc},
                {DISPLAY_OPTION_DRC,     strings.drc_screen_only}};

        settings.add(WUPSConfigItemMultipleValues::CreateFromValue(AUTOLAUNCH_DRC_SUPPORTED_CONFIG_ID,
                                                                   strings.autolaunch_drc_supported,
                                                                   DEFAULT_AUTOLAUNCH_DRC_SUPPORTED_VALUE,
                                                                   gAutolaunchDRCSupported,
                                                                   autolaunchDRCSupportedValues,
                                                                   &multipleValueItemCallback));

        // Autolaunch (GamePad not supported)
        const WUPSConfigItemMultipleValues::ValuePair autolaunchNoDRCSupportValues[] = {
                {DISPLAY_OPTION_CHOOSE, strings.select_each_time},
                {DISPLAY_OPTION_TV,     strings.tv_only},
                {DISPLAY_OPTION_BOTH,   strings.tv_and_drc},
                {DISPLAY_OPTION_DRC,    strings.drc_screen_only}};

        settings.add(WUPSConfigItemMultipleValues::CreateFromValue(AUTOLAUNCH_NO_DRC_SUPPORT_CONFIG_ID,
                                                                   strings.autolaunch_drc_not_supported,
                                                                   DEFAULT_AUTOLAUNCH_NO_DRC_SUPPORT_VALUE,
                                                                   gAutolaunchNoDRCSupport,
                                                                   autolaunchNoDRCSupportValues,
                                                                   &multipleValueItemCallback));

        // Set resolution
        const WUPSConfigItemMultipleValues::ValuePair setResolutionValues[] = {
                {SET_RESOLUTION_NONE,    strings.same_as_wii_u},
                {SET_RESOLUTION_480P,    strings.p480},
                {SET_RESOLUTION_480P_43, strings.p480_43},
                {SET_RESOLUTION_720P,    strings.p720},
                {SET_RESOLUTION_480I,    strings.i480},
                {SET_RESOLUTION_480I_43, strings.i480_43},
                {SET_RESOLUTION_576I,    strings.i576},
                {SET_RESOLUTION_576I_43, strings.i576_43},
                {SET_RESOLUTION_1080I,   strings.i1080},
                {SET_RESOLUTION_1080P,   strings.p1080}};

        settings.add(WUPSConfigItemMultipleValues::CreateFromValue(SET_RESOLUTION_CONFIG_ID,
                                                                   strings.set_resolution,
                                                                   DEFAULT_SET_RESOLUTION_VALUE,
                                                                   gSetResolution,
                                                                   setResolutionValues,
                                                                   &multipleValueItemCallback));

        // Display options order
        const WUPSConfigItemMultipleValues::ValuePair displayOptionsOrderValues[] = {
                {DISPLAY_OPTIONS_ORDER_DEFAULT, strings.order_default},
                {DISPLAY_OPTIONS_ORDER_RECENT,  strings.order_recent}};

        settings.add(WUPSConfigItemMultipleValues::CreateFromValue(DISPLAY_OPTIONS_ORDER_CONFIG_ID,
                                                                   strings.display_options_order,
                                                                   DEFAULT_DISPLAY_OPTIONS_ORDER_VALUE,
                                                                   gDisplayOptionsOrder,
                                                                   displayOptionsOrderValues,
                                                                   &multipleValueItemCallback));

        // Use custom dialogs
        settings.add(WUPSConfigItemBoolean::Create(USE_CUSTOM_DIALOGS_CONFIG_ID,
                                                   strings.use_custom_dialogs,
                                                   DEFAULT_USE_CUSTOM_DIALOGS_VALUE,
                                                   gUseCustomDialogs,
                                                   &boolItemCallback));

        // Separator
        settings.add(WUPSConfigItemStub::Create(" "));

        // Help text
        settings.add(WUPSConfigItemStub::Create(strings.override_autolaunch));

        root.add(std::move(settings));

        // Category: Wii Mode settings
        auto wiiMenuSettings = WUPSConfigCategory::Create(strings.wii_mode_settings);

        // Wii Mode set resolution
        wiiMenuSettings.add(WUPSConfigItemMultipleValues::CreateFromValue(WII_MENU_SET_RESOLUTION_CONFIG_ID,
                                                                          strings.set_resolution,
                                                                          DEFAULT_WII_MENU_SET_RESOLUTION_VALUE,
                                                                          gWiiMenuSetResolution,
                                                                          setResolutionValues,
                                                                          &multipleValueItemCallback));

        root.add(std::move(wiiMenuSettings));

        // Category: WUHB Forwarder settings
        auto forwarderSettings = WUPSConfigCategory::Create(strings.wuhb_forwarder_settings);

        // Override display
        const WUPSConfigItemMultipleValues::ValuePair forwarderDisplayOverrideValues[] = {
                {DISPLAY_OPTION_CHOOSE, strings.do_not_override},
                {DISPLAY_OPTION_TV,     strings.tv_only},
                {DISPLAY_OPTION_BOTH,   strings.tv_and_drc},
                {DISPLAY_OPTION_DRC,    strings.drc_screen_only}};

        forwarderSettings.add(WUPSConfigItemMultipleValues::CreateFromValue(FORWARDER_DISPLAY_OVERRIDE_CONFIG_ID,
                                                                            strings.override_display,
                                                                            DEFAULT_FORWARDER_DISPLAY_OVERRIDE,
                                                                            gForwarderDisplayOverride,
                                                                            forwarderDisplayOverrideValues,
                                                                            &multipleValueItemCallback));

        root.add(std::move(forwarderSettings));

        // Category: Other settings
        auto otherSettings = WUPSConfigCategory::Create(strings.other_settings);

        // Notification theme
        const WUPSConfigItemMultipleValues::ValuePair notificationThemeValues[] = {
                {NOTIFICATION_THEME_OFF,   strings.theme_off},
                {NOTIFICATION_THEME_DARK,  strings.theme_dark},
                {NOTIFICATION_THEME_LIGHT, strings.theme_light}};

        otherSettings.add(WUPSConfigItemMultipleValues::CreateFromValue(NOTIFICATION_THEME_CONFIG_ID,
                                                                        strings.notification_theme,
                                                                        DEFAULT_NOTIFICATION_THEME_VALUE,
                                                                        gNotificationTheme,
                                                                        notificationThemeValues,
                                                                        &multipleValueItemCallback));

        // Preserve SYSCONF
        otherSettings.add(WUPSConfigItemBoolean::Create(PRESERVE_SYSCONF_CONFIG_ID,
                                                        strings.preserve_sysconf,
                                                        DEFAULT_PRESERVE_SYSCONF_VALUE,
                                                        gPreserveSysconf,
                                                        &boolItemCallback));

        // Permanent Internet Settings
        otherSettings.add(WUPSConfigItemBoolean::Create(PERMANENT_NET_CONFIG_CONFIG_ID,
                                                        strings.permanent_wii_internet_settings,
                                                        DEFAULT_PERMANENT_NET_CONFIG_VALUE,
                                                        gPermanentNetConfig,
                                                        &boolItemCallback));

        // Language
        constexpr WUPSConfigItemMultipleValues::ValuePair languageValues[] = {
                {Language::English, "English"},
                {Language::Spanish, "Español"},
                {Language::System,  "System"}};

        otherSettings.add(WUPSConfigItemMultipleValues::CreateFromValue(LANGUAGE_CONFIG_ID,
                                                                        "Language", /* intentionally not translated */
                                                                        Language::System,
                                                                        sLanguageSetting,
                                                                        languageValues,
                                                                        &languageChangedCallback));

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

    WUPSStorageAPI::GetOrStoreDefault(PRESERVE_SYSCONF_CONFIG_ID, gPreserveSysconf, DEFAULT_PRESERVE_SYSCONF_VALUE);

    WUPSStorageAPI::GetOrStoreDefault(PERMANENT_NET_CONFIG_CONFIG_ID, gPermanentNetConfig, DEFAULT_PERMANENT_NET_CONFIG_VALUE);

    WUPSStorageAPI::GetOrStoreDefault(LANGUAGE_CONFIG_ID, sLanguageSetting, Language::System);

    // Set the language that's currently used
    setLanguage(sLanguageSetting);
}
