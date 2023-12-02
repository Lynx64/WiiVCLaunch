#include "config.h"
#include "globals.hpp"
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemMultipleValues.h>
#include <string_view>

WUPS_USE_STORAGE("WiiVCLaunch");

void initConfig()
{
    // Open storage to read values
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        //failed to open storage - use default values
    } else {
        // Try to get value from storage
        if (WUPS_GetInt(nullptr, "gAutolaunchDRCSupported", reinterpret_cast<int32_t *>(&gAutolaunchDRCSupported)) != WUPS_STORAGE_ERROR_SUCCESS) {
            // Add the value to the storage if it's missing
            gAutolaunchDRCSupported = DISPLAY_OPTION_CHOOSE;
            WUPS_StoreInt(nullptr, "gAutolaunchDRCSupported", (int32_t) gAutolaunchDRCSupported);
        }

        if (WUPS_GetInt(nullptr, "gAutolaunchNoDRCSupport", reinterpret_cast<int32_t *>(&gAutolaunchNoDRCSupport)) != WUPS_STORAGE_ERROR_SUCCESS) {
            gAutolaunchNoDRCSupport = DISPLAY_OPTION_CHOOSE;
            WUPS_StoreInt(nullptr, "gAutolaunchNoDRCSupport", (int32_t) gAutolaunchNoDRCSupport);
        }

        if (WUPS_GetInt(nullptr, "gDisplayOptionsOrder", reinterpret_cast<int32_t *>(&gDisplayOptionsOrder)) != WUPS_STORAGE_ERROR_SUCCESS) {
            gDisplayOptionsOrder = DISPLAY_OPTIONS_ORDER_RECENT;
            WUPS_StoreInt(nullptr, "gDisplayOptionsOrder", (int32_t) gDisplayOptionsOrder);
        }

        if (WUPS_GetInt(nullptr, "gSetResolution", reinterpret_cast<int32_t *>(&gSetResolution)) != WUPS_STORAGE_ERROR_SUCCESS) {
            gSetResolution = SET_RESOLUTION_NONE;
            WUPS_StoreInt(nullptr, "gSetResolution", (int32_t) gSetResolution);
        }

        if (WUPS_GetBool(nullptr, "gUseCustomDialogs", &gUseCustomDialogs) != WUPS_STORAGE_ERROR_SUCCESS) {
            gUseCustomDialogs = true;
            WUPS_StoreBool(nullptr, "gUseCustomDialogs", gUseCustomDialogs);
        }

        // Close storage
        WUPS_CloseStorage();
    }
}

void boolItemCallback(ConfigItemBoolean *item, bool newValue)
{
    if (item && item->configId) {
        if (std::string_view(item->configId) == "gUseCustomDialogs") {
            gUseCustomDialogs = newValue;
            WUPS_StoreBool(nullptr, item->configId, gUseCustomDialogs);
        }
    }
}

void multipleValueItemCallback(ConfigItemMultipleValues *item, uint32_t newValue)
{
    if (item && item->configId) {
        if (std::string_view(item->configId) == "gAutolaunchDRCSupported") {
            gAutolaunchDRCSupported = newValue;
            WUPS_StoreInt(nullptr, item->configId, (int32_t) gAutolaunchDRCSupported);
        } else if (std::string_view(item->configId) == "gAutolaunchNoDRCSupport") {
            gAutolaunchNoDRCSupport = newValue;
            WUPS_StoreInt(nullptr, item->configId, (int32_t) gAutolaunchNoDRCSupport);
        } else if (std::string_view(item->configId) == "gDisplayOptionsOrder") {
            gDisplayOptionsOrder = newValue;
            WUPS_StoreInt(nullptr, item->configId, (int32_t) gDisplayOptionsOrder);
        } else if (std::string_view(item->configId) == "gSetResolution") {
            gSetResolution = newValue;
            WUPS_StoreInt(nullptr, item->configId, (int32_t) gSetResolution);
        }
    }
}

WUPS_GET_CONFIG()
{
    WUPSConfigHandle config;
    WUPSConfig_CreateHandled(&config, "\ue067 VC Launch");

    // Open the storage
    WUPSStorageError storageRes = WUPS_OpenStorage();
    if (storageRes != WUPS_STORAGE_ERROR_SUCCESS) {
        //failed to open storage
        if (storageRes == WUPS_STORAGE_ERROR_ALREADY_OPENED) {
            WUPSConfigCategoryHandle errorInfo1;
            WUPSConfig_AddCategoryByNameHandled(config, "Cannot edit config right now", &errorInfo1);
            return config;
        }

        WUPSConfigCategoryHandle errorInfo1;
        WUPSConfig_AddCategoryByNameHandled(config, "Error opening storage", &errorInfo1);

        WUPSConfigCategoryHandle errorInfo2;
        WUPSConfig_AddCategoryByNameHandled(config, "Try deleting location_of_this_plugin/config/WiiVCLaunch.json", &errorInfo2);
        return config;
    }

    // Category: Settings
    WUPSConfigCategoryHandle settings;
    WUPSConfig_AddCategoryByNameHandled(config, "Settings", &settings);

    // Autolaunch (GamePad supported)
    ConfigItemMultipleValuesPair autolaunchDRCSupportedValues[5];
    autolaunchDRCSupportedValues[0].value       = DISPLAY_OPTION_CHOOSE;
    autolaunchDRCSupportedValues[0].valueName   = (char *) "Select each time";

    autolaunchDRCSupportedValues[1].value       = DISPLAY_OPTION_USE_DRC;
    autolaunchDRCSupportedValues[1].valueName   = (char *) "Use \ue087 as controller";

    autolaunchDRCSupportedValues[2].value       = DISPLAY_OPTION_TV;
    autolaunchDRCSupportedValues[2].valueName   = (char *) "TV Only";

    autolaunchDRCSupportedValues[3].value       = DISPLAY_OPTION_BOTH;
    autolaunchDRCSupportedValues[3].valueName   = (char *) "TV and \ue087";

    autolaunchDRCSupportedValues[4].value       = DISPLAY_OPTION_DRC;
    autolaunchDRCSupportedValues[4].valueName   = (char *) "\ue087 screen only";

    int32_t defaultIndex = 0;
    int32_t curIndex = 0;
    for (auto &cur : autolaunchDRCSupportedValues) {
        if (cur.value == gAutolaunchDRCSupported) {
            defaultIndex = curIndex;
            break;
        }
        curIndex++;
    }

    WUPSConfigItemMultipleValues_AddToCategoryHandled(config, settings, "gAutolaunchDRCSupported", "Autolaunch (\ue087 supported)", defaultIndex, autolaunchDRCSupportedValues,
                                                     5, &multipleValueItemCallback);

    // Autolaunch (GamePad not supported)
    ConfigItemMultipleValuesPair autolaunchNoDRCSupportValues[4];
    autolaunchNoDRCSupportValues[0].value       = DISPLAY_OPTION_CHOOSE;
    autolaunchNoDRCSupportValues[0].valueName   = (char *) "Select each time";

    autolaunchNoDRCSupportValues[1].value       = DISPLAY_OPTION_TV;
    autolaunchNoDRCSupportValues[1].valueName   = (char *) "TV Only";

    autolaunchNoDRCSupportValues[2].value       = DISPLAY_OPTION_BOTH;
    autolaunchNoDRCSupportValues[2].valueName   = (char *) "TV and \ue087";

    autolaunchNoDRCSupportValues[3].value       = DISPLAY_OPTION_DRC;
    autolaunchNoDRCSupportValues[3].valueName   = (char *) "\ue087 screen only";

    defaultIndex = 0;
    curIndex = 0;
    for (auto &cur : autolaunchNoDRCSupportValues) {
        if (cur.value == gAutolaunchNoDRCSupport) {
            defaultIndex = curIndex;
            break;
        }
        curIndex++;
    }

    WUPSConfigItemMultipleValues_AddToCategoryHandled(config, settings, "gAutolaunchNoDRCSupport", "Autolaunch (\ue087 not supported)", defaultIndex, autolaunchNoDRCSupportValues,
                                                     4, &multipleValueItemCallback);

    // Set resolution
    ConfigItemMultipleValuesPair setResolutionValues[4];
    setResolutionValues[0].value       = SET_RESOLUTION_NONE;
    setResolutionValues[0].valueName   = (char *) "Same as Wii U";

    setResolutionValues[1].value       = SET_RESOLUTION_480P;
    setResolutionValues[1].valueName   = (char *) "480p";

    setResolutionValues[2].value       = SET_RESOLUTION_480P_43;
    setResolutionValues[2].valueName   = (char *) "480p (4:3)";

    setResolutionValues[3].value       = SET_RESOLUTION_720P;
    setResolutionValues[3].valueName   = (char *) "720p";

    defaultIndex = 0;
    curIndex = 0;
    for (auto &cur : setResolutionValues) {
        if (cur.value == gSetResolution) {
            defaultIndex = curIndex;
            break;
        }
        curIndex++;
    }

    WUPSConfigItemMultipleValues_AddToCategoryHandled(config, settings, "gSetResolution", "Set resolution", defaultIndex, setResolutionValues,
                                                     4, &multipleValueItemCallback);

    // Display options order
    ConfigItemMultipleValuesPair displayOptionsOrderValues[2];
    displayOptionsOrderValues[0].value       = DISPLAY_OPTIONS_ORDER_DEFAULT;
    displayOptionsOrderValues[0].valueName   = (char *) "Default";

    displayOptionsOrderValues[1].value       = DISPLAY_OPTIONS_ORDER_RECENT;
    displayOptionsOrderValues[1].valueName   = (char *) "Recent";

    defaultIndex = 0;
    curIndex = 0;
    for (auto &cur : displayOptionsOrderValues) {
        if (cur.value == gDisplayOptionsOrder) {
            defaultIndex = curIndex;
            break;
        }
        curIndex++;
    }

    WUPSConfigItemMultipleValues_AddToCategoryHandled(config, settings, "gDisplayOptionsOrder", "Display options order", defaultIndex, displayOptionsOrderValues,
                                                     2, &multipleValueItemCallback);

    // Category: Advanced options
    WUPSConfigCategoryHandle advancedOptions;
    WUPSConfig_AddCategoryByNameHandled(config, "Advanced options", &advancedOptions);

    WUPSConfigItemBoolean_AddToCategoryHandled(config, advancedOptions, "gUseCustomDialogs", "Use custom dialogs", gUseCustomDialogs, &boolItemCallback);

    return config;
}

WUPS_CONFIG_CLOSED()
{
    // Save all changes
    WUPS_CloseStorage();
}
