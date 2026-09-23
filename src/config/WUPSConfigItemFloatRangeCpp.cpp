#include "config/WUPSConfigItemFloatRange.h"

std::optional<WUPSConfigItemFloatRange> WUPSConfigItemFloatRange::Create(
        std::optional<std::string> identifier,
        std::string_view displayName,
        float defaultValue, float currentValue,
        float minValue, float maxValue,
        float stepValue,
        FloatRangeValueChangedCallback valuesChangedCallback,
        WUPSConfigAPIStatus &err) noexcept {
    WUPSConfigItemHandle itemHandle;
    if ((err = WUPSConfigItemFloatRange_Create(
            identifier ? identifier->c_str() : nullptr,
            displayName.data(),
            defaultValue, currentValue,
            minValue, maxValue,
            stepValue,
            valuesChangedCallback,
            &itemHandle)) != WUPSCONFIG_API_RESULT_SUCCESS) {
        return std::nullopt;
    }
    return WUPSConfigItemFloatRange(itemHandle);
}

WUPSConfigItemFloatRange WUPSConfigItemFloatRange::Create(
        std::optional<std::string> identifier,
        std::string_view displayName,
        float defaultValue, float currentValue,
        float minValue, float maxValue,
        float stepValue,
        FloatRangeValueChangedCallback valuesChangedCallback) {
    WUPSConfigAPIStatus err = WUPSCONFIG_API_RESULT_UNKNOWN_ERROR;
    auto result             = Create(std::move(identifier), displayName, defaultValue, currentValue, minValue, maxValue, stepValue, valuesChangedCallback, err);
    if (!result) {
        throw std::runtime_error(std::string("Failed to create WUPSConfigItemFloatRange: ").append(WUPSConfigAPI_GetStatusStr(err)));
    }
    return std::move(*result);
}
