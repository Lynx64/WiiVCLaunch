#include "lang.h"
#include "logger.h"

#include <coreinit/userconfig.h>

#include <optional>

static Language sLanguage = Language::English;

static constexpr TranslatedStrings es_PE = {
#include "es_PE.lang"
};
static constexpr TranslatedStrings en_GB = {
#include "en_GB.lang"
};

// https://github.com/PretendoNetwork/Inkay/blob/68afedb5ee07832cd0b57be7f2ea3fd092f80233/common/sysconfig.cpp#L28
Language getSystemLanguage() {
    static std::optional<Language> cachedLanguage{};
    if (cachedLanguage)
        return *cachedLanguage;

    UCHandle handle = UCOpen();
    if (handle < 0) {
        DEBUG_FUNCTION_LINE_ERR("Error opening UC: %d", handle);
        return Language::English;
    }

    Language language;

    alignas(0x40) UCSysConfig settings = {
            .name = "cafe.language",
            .access = 0,
            .dataType = UC_DATATYPE_UNSIGNED_INT,
            .error = UC_ERROR_OK,
            .dataSize = sizeof(language),
            .data = &language,
    };

    UCError err = UCReadSysConfig(handle, 1, &settings);
    if (err != UC_ERROR_OK) {
        DEBUG_FUNCTION_LINE_ERR("Error reading UC: %d", err);
        UCClose(handle);
        return Language::English;
    }

    UCClose(handle);
    DEBUG_FUNCTION_LINE_VERBOSE("System language found: %d", language);
    cachedLanguage = language;
    return language;
}

void setLanguage(const Language &newLanguage) {
    if (newLanguage == Language::System) {
        sLanguage = getSystemLanguage();
    } else {
        sLanguage = newLanguage;
    }
}

const TranslatedStrings& getTranslatedStrings() {
    switch (sLanguage) {
        case Language::Spanish:
            return es_PE;
        case Language::English:
        default:
            return en_GB;
    }
}
