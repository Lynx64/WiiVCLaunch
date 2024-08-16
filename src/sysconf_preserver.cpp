#include "logger.h"
#include <filesystem>
#include <mocha/mocha.h>
#include <notifications/notifications.h>
#include <wups/storage.h>

// adapted from https://github.com/wiiu-env/ftpiiu_plugin/blob/00191956b81965733ad26dc34f9569201b79aff7/source/wiiu/platform.cpp#L77
static MochaUtilsStatus MountWrapper(const char *mount, const char *dev, const char *mountTo)
{
    auto res = Mocha_MountFS(mount, dev, mountTo);
    if (res == MOCHA_RESULT_ALREADY_EXISTS) {
        res = Mocha_MountFS(mount, nullptr, mountTo);
    }
    if (res != MOCHA_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to mount %s: %s (%d)", mount, Mocha_GetStatusStr(res), res);
    }
    return res;
}
//TODO: organise all this code
void backupSysconf()
{
    auto mochaInitRes = Mocha_InitLibrary();
    if (mochaInitRes != MOCHA_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init libmocha: %s (%d)", Mocha_GetStatusStr(mochaInitRes), mochaInitRes);
        return;
    }

    if (MountWrapper("slccmpt01", "/dev/slccmpt01", "/vol/storage_slccmpt01") != MOCHA_RESULT_SUCCESS) {
        return;
    }

    std::error_code ec; //TODO: catch exceptions
    bool copyRes = std::filesystem::copy_file("/vol/storage_slccmpt01/shared2/sys/SYSCONF",
                                              "/vol/external01/wiiu/SYSCONF",
                                              std::filesystem::copy_options::overwrite_existing,
                                              ec);
    DEBUG_FUNCTION_LINE("%d %s", copyRes, ec.message().c_str());
    NotificationModule_AddInfoNotification(ec.message().c_str());
    Mocha_UnmountFS("slccmpt01");
    Mocha_DeInitLibrary();
    //TODO: check copy was successful before storing this
    WUPSStorageAPI::Store("restoreSysconf", true);
    WUPSStorageAPI::SaveStorage();
}

void restoreSysconf()
{
    bool restore = false;
    WUPSStorageAPI::Get("restoreSysconf", restore);
    if (!restore) return;
    WUPSStorageAPI::Store("restoreSysconf", false);
    WUPSStorageAPI::SaveStorage();
    
    try {
        auto fileSize = std::filesystem::file_size("/vol/external01/wiiu/SYSCONF");
        if (fileSize != 0x4000) {
            DEBUG_FUNCTION_LINE_ERR("SD's SYSCONF is wrong size, should be 16,384 bytes");
            return;
        }
    } catch (const std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("%s", e.what());
        return;
    }
    
    auto mochaInitRes = Mocha_InitLibrary();
    if (mochaInitRes != MOCHA_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("Failed to init libmocha: %s (%d)", Mocha_GetStatusStr(mochaInitRes), mochaInitRes);
        return;
    }

    if (MountWrapper("slccmpt01", "/dev/slccmpt01", "/vol/storage_slccmpt01") != MOCHA_RESULT_SUCCESS) {
        return;
    }

    std::error_code ec;
    //TODO: do some checks on the file before copying it
    bool copyRes = std::filesystem::copy_file("/vol/external01/wiiu/SYSCONF",
                                              "/vol/storage_slccmpt01/shared2/sys/SYSCONF",
                                              std::filesystem::copy_options::overwrite_existing,
                                              ec);
    DEBUG_FUNCTION_LINE("%d %s", copyRes, ec.message().c_str());
    Mocha_UnmountFS("slccmpt01");
    Mocha_DeInitLibrary();
}
