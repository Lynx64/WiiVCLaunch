#include "rpx_patch.h"
#include "logger.h"

// Copied from https://github.com/Project-Rose/RosePatcher/blob/c31a12778d5c8520751502e3205ce50c2c708acc/src/utils/patch.cpp#L21
uint32_t findMem(uint32_t start, uint32_t size, const char *originalVal, size_t originalValSize) {
    for (uint32_t addr = start; addr < start + size - originalValSize; addr++) {
        if (memcmp(originalVal, (void *) addr, originalValSize) == 0) {
            return addr;
        }
    }
    return 0;
}

// Copied from https://github.com/Project-Rose/RosePatcher/blob/c31a12778d5c8520751502e3205ce50c2c708acc/src/utils/patch.cpp#L78
bool getRplInfo(std::vector<OSDynLoad_NotifyData> &rpls) {
    int32_t numRpls = OSDynLoad_GetNumberOfRPLs();

    DEBUG_FUNCTION_LINE("%d RPL(s) running", numRpls);

    if (numRpls == 0) {
        return false;
    }

    rpls.resize(numRpls);

    bool ret = OSDynLoad_GetRPLInfo(0, numRpls, rpls.data());

    return ret;
}

// Copied from https://github.com/Project-Rose/RosePatcher/blob/c31a12778d5c8520751502e3205ce50c2c708acc/src/utils/patch.cpp#L94
bool findRpl(const std::string_view &name, OSDynLoad_NotifyData &foundRpl) {
    std::vector<OSDynLoad_NotifyData> rplInfo;
    if (!getRplInfo(rplInfo)) {
        DEBUG_FUNCTION_LINE("Failed to get RPL info");
        return false;
    }

    DEBUG_FUNCTION_LINE("Got RPL info");

    for (const auto &rpl : rplInfo) {
        if (rpl.name == nullptr || rpl.name[0] == '\0') {
            continue;
        }
        if (std::string_view(rpl.name).ends_with(name)) {
            foundRpl = rpl;
            return true;
        }
    }

    return false;
}
