#pragma once

#include <string_view>

enum Language {
    Japanese = 0,
    English = 1,
    French = 2,
    German = 3,
    Italian = 4,
    Spanish = 5,
    ChineseSimplified = 6,
    Korean = 7,
    Dutch = 8,
    Portuguese = 9,
    Russian = 10,
    ChineseTraditional = 11,
    Invalid = 12,
    System = 13
};

struct TranslatedStrings {
    const char *     use_gamepad_as_controller;
    const char *     tv_and_gamepad;
    const char *     gamepad_screen_only;
    const char16_t * use_drc_as_controller2;
    const char16_t * tv_only2;
    const char16_t * tv_and_drc2;
    const char16_t * drc_screen_only2;
    const char *     autolaunching;
    const char16_t * select_a_display_option_more_options;
    const char16_t * select_a_display_option_detect_tv;
    const char *     gamepad_sensor_bar_enabled;

    std::string_view wii_vc_settings;
    std::string_view wii_mode_settings;
    std::string_view wuhb_forwarder_settings;
    std::string_view other_settings;

    std::string_view autolaunch_drc_supported;
    std::string_view autolaunch_drc_not_supported;
    std::string_view set_resolution;
    std::string_view display_options_order;
    std::string_view use_custom_dialogs;
    std::string_view override_autolaunch;

    std::string_view override_display;

    std::string_view notification_theme;
    std::string_view preserve_sysconf;
    std::string_view permanent_wii_internet_settings;

    std::string_view select_each_time;
    std::string_view use_drc_as_controller;
    std::string_view tv_only;
    std::string_view tv_and_drc;
    std::string_view drc_screen_only;

    std::string_view order_default;
    std::string_view order_recent;

    std::string_view do_not_override;

    std::string_view theme_off;
    std::string_view theme_dark;
    std::string_view theme_light;
};

void setLanguage(const Language &newLanguage);
const TranslatedStrings& getTranslatedStrings();
