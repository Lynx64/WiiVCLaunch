#include "config/WUPSConfigItemFloatRange.h"

#include <wups/config_api.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

void WUPSConfigItemFloatRange_onCloseCallback(void *context) {
    auto *item = (ConfigItemFloatRange *) context;
    if (item->valueAtCreation != item->value && item->valueChangedCallback != nullptr) {
        ((FloatRangeValueChangedCallback) item->valueChangedCallback)(item, item->value);
    }
}

void WUPSConfigItemFloatRange_onInput(void *context, WUPSConfigSimplePadData input) {
    auto *item = (ConfigItemFloatRange *) context;

    if (input.buttons_d & WUPS_CONFIG_BUTTON_LEFT) {
        item->value -= item->stepValue;
    } else if (input.buttons_d & WUPS_CONFIG_BUTTON_RIGHT) {
        item->value += item->stepValue;
    } else if (input.buttons_d & WUPS_CONFIG_BUTTON_L) {
        item->value -= item->stepValue * 10.0f;
    } else if (input.buttons_d & WUPS_CONFIG_BUTTON_R) {
        item->value += item->stepValue * 10.0f;
    }

    if (item->value < item->minValue) {
        item->value = item->minValue;
    } else if (item->value > item->maxValue) {
        item->value = item->maxValue;
    }
}

int32_t WUPSConfigItemFloatRange_getCurrentValueDisplay(void *context, char *out_buf, int32_t out_size) {
    auto *item = (ConfigItemFloatRange *) context;
    snprintf(out_buf, out_size, "%.1f", item->value);
    return 0;
}

int32_t WUPSConfigItemFloatRange_getCurrentValueSelectedDisplay(void *context, char *out_buf, int32_t out_size) {
    auto *item = (ConfigItemFloatRange *) context;
    if (item->value == item->minValue) {
        snprintf(out_buf, out_size, "  %.1f >", item->value);
    } else if (item->value == item->maxValue) {
        snprintf(out_buf, out_size, "< %.1f  ", item->value);
    } else {
        snprintf(out_buf, out_size, "< %.1f >", item->value);
    }
    return 0;
}

void WUPSConfigItemFloatRange_restoreDefault(void *context) {
    auto *item  = (ConfigItemFloatRange *) context;
    item->value = item->defaultValue;
}

static void WUPSConfigItemFloatRange_Cleanup(ConfigItemFloatRange *item) {
    if (!item) {
        return;
    }
    free((void *) item->identifier);
    free(item);
}

void WUPSConfigItemFloatRange_onDelete(void *context) {
    WUPSConfigItemFloatRange_Cleanup((ConfigItemFloatRange *) context);
}

extern "C" WUPSConfigAPIStatus
WUPSConfigItemFloatRange_Create(const char *identifier,
                                const char *displayName,
                                float defaultValue, float currentValue,
                                float minValue, float maxValue,
                                float stepValue,
                                FloatRangeValueChangedCallback callback,
                                WUPSConfigItemHandle *outHandle) {
    if (outHandle == nullptr) {
        return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
    }
    if (maxValue < minValue || defaultValue < minValue || defaultValue > maxValue || currentValue < minValue || currentValue > maxValue) {
        return WUPSCONFIG_API_RESULT_INVALID_ARGUMENT;
    }
    *outHandle = {};
    auto *item = (ConfigItemFloatRange *) malloc(sizeof(ConfigItemFloatRange));
    if (item == nullptr) {
        return WUPSCONFIG_API_RESULT_OUT_OF_MEMORY;
    }

    if (identifier != nullptr) {
        item->identifier = strdup(identifier);
    } else {
        item->identifier = nullptr;
    }

    item->defaultValue         = defaultValue;
    item->value                = currentValue;
    item->valueAtCreation      = currentValue;
    item->minValue             = minValue;
    item->maxValue             = maxValue;
    item->stepValue            = stepValue;
    item->valueChangedCallback = (void *) callback;

    WUPSConfigAPIItemCallbacksV2 callbacks = {
            .getCurrentValueDisplay         = &WUPSConfigItemFloatRange_getCurrentValueDisplay,
            .getCurrentValueSelectedDisplay = &WUPSConfigItemFloatRange_getCurrentValueSelectedDisplay,
            .onSelected                     = nullptr,
            .restoreDefault                 = &WUPSConfigItemFloatRange_restoreDefault,
            .isMovementAllowed              = nullptr,
            .onCloseCallback                = &WUPSConfigItemFloatRange_onCloseCallback,
            .onInput                        = &WUPSConfigItemFloatRange_onInput,
            .onInputEx                      = nullptr,
            .onDelete                       = &WUPSConfigItemFloatRange_onDelete};

    WUPSConfigAPIItemOptionsV2 options = {
            .displayName = displayName,
            .context     = item,
            .callbacks   = callbacks};

    WUPSConfigAPIStatus err;
    if ((err = WUPSConfigAPI_Item_Create(options, &item->handle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        WUPSConfigItemFloatRange_Cleanup(item);
        return err;
    }

    *outHandle = item->handle;

    return WUPSCONFIG_API_RESULT_SUCCESS;
}

extern "C" WUPSConfigAPIStatus
WUPSConfigItemFloatRange_AddToCategory(WUPSConfigCategoryHandle cat,
                                       const char *identifier,
                                       const char *displayName,
                                       float defaultValue, float currentValue,
                                       float minValue, float maxValue,
                                       float stepValue,
                                       FloatRangeValueChangedCallback callback) {
    WUPSConfigItemHandle itemHandle;
    WUPSConfigAPIStatus res;
    if ((res = WUPSConfigItemFloatRange_Create(identifier,
                                               displayName,
                                               defaultValue, currentValue,
                                               minValue, maxValue,
                                               stepValue,
                                               callback,
                                               &itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return res;
    }

    if ((res = WUPSConfigAPI_Category_AddItem(cat, itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        WUPSConfigAPI_Item_Destroy(itemHandle);
        return res;
    }
    return WUPSCONFIG_API_RESULT_SUCCESS;
}
