#include "lang.h"
#include "logger.h"

#include <coreinit/mcp.h>
#include <coreinit/userconfig.h>

#include <optional>

static Language sLanguage = Language::English;

static constexpr TranslatedStrings pt_PT = {
#include "pt_PT.lang"
};
static constexpr TranslatedStrings de_DE = {
#include "de_DE.lang"
};
static constexpr TranslatedStrings es_PE = {
#include "es_PE.lang"
};
static constexpr TranslatedStrings es_ES = {
#include "es_ES.lang"
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
    UCClose(handle);
    if (err != UC_ERROR_OK) {
        DEBUG_FUNCTION_LINE_ERR("Error reading UC: %d", err);
        return Language::English;
    }

    DEBUG_FUNCTION_LINE_VERBOSE("System language found: %d", language);
    cachedLanguage = language;
    return language;
}

MCPRegion getSystemRegion() {
    static std::optional<MCPRegion> cachedRegion{};
    if (cachedRegion)
        return *cachedRegion;

    int32_t handle = MCP_Open();
    if (handle < 0) {
        DEBUG_FUNCTION_LINE_ERR("Error opening MCP: %d", handle);
        return MCPRegion::MCP_REGION_EUROPE;
    }

    alignas(0x40) MCPSysProdSettings settings{};
    MCPError err = MCP_GetSysProdSettings(handle, &settings);
    MCP_Close(handle);
    if (err != 0) {
        DEBUG_FUNCTION_LINE_ERR("Error GetSysProdSettings: %d", err);
        return MCPRegion::MCP_REGION_EUROPE;
    }

    MCPRegion region = settings.product_area;
    cachedRegion = region;
    return region;
}

void setLanguage(Language newLanguage) {
    if (newLanguage == Language::System) {
        newLanguage = getSystemLanguage();
        MCPRegion region = getSystemRegion();
        switch (newLanguage) {
            case Language::Spanish:
                if (region != MCPRegion::MCP_REGION_USA) {
                    newLanguage = Language::SpanishSpain;
                }
                break;
            default:
                break;
        }
    }
    sLanguage = newLanguage;
}

const TranslatedStrings& getTranslatedStrings() {
    switch (sLanguage) {
        case Language::Portuguese:
            return pt_PT;
        case Language::German:
            return de_DE;
        case Language::Spanish:
            return es_PE;
        case Language::SpanishSpain:
            return es_ES;
        case Language::English:
        default:
            return en_GB;
    }
}
