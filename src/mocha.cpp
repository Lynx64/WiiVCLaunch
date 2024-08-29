#include "logger.h"
#include "mocha.h"

// adapted from https://github.com/wiiu-env/ftpiiu_plugin/blob/00191956b81965733ad26dc34f9569201b79aff7/source/wiiu/platform.cpp#L77-L95
MochaUtilsStatus MountWrapper(const char *mount, const char *dev, const char *mountTo)
{
    auto res = Mocha_MountFS(mount, dev, mountTo);
    if (res == MOCHA_RESULT_ALREADY_EXISTS) {
        res = Mocha_MountFS(mount, nullptr, mountTo);
    }
    if (res != MOCHA_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to mount %s: %s (%d)",
                                mount,
                                Mocha_GetStatusStr(res),
                                res);
    }
    return res;
}

MochaUtilsStatus initMocha()
{
    auto mochaInitRes = Mocha_InitLibrary();
    if (mochaInitRes != MOCHA_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init libmocha: %s (%d)",
                                Mocha_GetStatusStr(mochaInitRes),
                                mochaInitRes);
    }
    return mochaInitRes;
}
