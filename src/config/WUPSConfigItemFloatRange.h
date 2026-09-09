#pragma once

#include <wups/config_api.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ConfigItemFloatRange {
    WUPSConfigItemHandle handle;
    const char *identifier;
    float defaultValue;
    float value;
    float valueAtCreation;
    float minValue;
    float maxValue;
    float stepValue;
    void *valueChangedCallback;
} ConfigItemFloatRange;

typedef void (*FloatRangeValueChangedCallback)(ConfigItemFloatRange *, float);

WUPSConfigAPIStatus
WUPSConfigItemFloatRange_Create(const char *identifier,
                                const char *displayName,
                                float defaultValue, float currentValue,
                                float minValue, float maxValue,
                                float stepValue,
                                FloatRangeValueChangedCallback callback,
                                WUPSConfigItemHandle *outHandle);

/**
 * \brief Adds a float range configuration item to a category.
 *
 * This function creates a new ConfigItemFloatRange item and adds it to the specified category.
 * The item represents a float value within a specified range, and allows the user to modify the value.
 *
 * \param cat                    The category handle to which the item should be added.
 * \param identifier             Optional identifier for the item. Can be NULL.
 * \param displayName            The display name for the item.
 * \param defaultValue           The default value for the item.
 * \param currentValue           The current value for the item.
 * \param minValue               The minimum value allowed for the item.
 * \param maxValue               The maximum value allowed for the item.
 * \param stepValue              The amount to increment/decrement the value each button press.
 * \param callback               A callback function that will be called when the config menu closes and the value of the item has been changed.
 *
 * \return                       Returns true if the item was successfully added to the category, false otherwise.
 *
 * @note The defaultValue and currentValue must be within the specified range.
 */
WUPSConfigAPIStatus
WUPSConfigItemFloatRange_AddToCategory(WUPSConfigCategoryHandle cat,
                                       const char *identifier,
                                       const char *displayName,
                                       float defaultValue, float currentValue,
                                       float minValue, float maxValue,
                                       float stepValue,
                                       FloatRangeValueChangedCallback callback);

#ifdef __cplusplus
}
#endif


#if defined(__cplusplus) && __cplusplus >= 201703L
#include <wups/config/WUPSConfigItem.h>

#include <optional>
#include <stdexcept>
#include <string>

class WUPSConfigItemFloatRange : public WUPSConfigItem {
public:
    static std::optional<WUPSConfigItemFloatRange> Create(
            std::optional<std::string> identifier,
            std::string_view displayName,
            float defaultValue, float currentValue,
            float minValue, float maxValue,
            float stepValue,
            FloatRangeValueChangedCallback valuesChangedCallback,
            WUPSConfigAPIStatus &err) noexcept;

    static WUPSConfigItemFloatRange Create(
            std::optional<std::string> identifier,
            std::string_view displayName,
            float defaultValue, float currentValue,
            float minValue, float maxValue,
            float stepValue,
            FloatRangeValueChangedCallback valuesChangedCallback);

private:
    explicit WUPSConfigItemFloatRange(WUPSConfigItemHandle itemHandle) : WUPSConfigItem(itemHandle) {
    }
};

#endif
