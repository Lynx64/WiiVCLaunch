#include "sysconf_preserver.h"
#include "logger.h"
#include "mocha.h"

#include <wups/storage.h>

#include <filesystem>

const std::filesystem::path SYSCONF_SD_PATH = "/vol/external01/wiiu/SYSCONF";
const std::filesystem::path SYSCONF_NAND_PATH = "slccmpt01:/shared2/sys/SYSCONF";

void backupSysconf()
{
    if (initMocha() != MOCHA_RESULT_SUCCESS)
        return;

    if (MountWrapper("slccmpt01", "/dev/slccmpt01", "/vol/storage_slccmpt01") != MOCHA_RESULT_SUCCESS) {
        Mocha_DeInitLibrary();
        return;
    }

    std::error_code ec;
    try {
        if (std::filesystem::copy_file(SYSCONF_NAND_PATH,
                                       SYSCONF_SD_PATH,
                                       std::filesystem::copy_options::overwrite_existing,
                                       ec)) {
            WUPSStorageAPI::Store("restoreSysconf", true);
            WUPSStorageAPI::SaveStorage();
        } else if (ec) {
            DEBUG_FUNCTION_LINE_ERR("Copy file failed: %s", ec.message().c_str());
        }
    } catch (const std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Copy file exception: %s", e.what());
    }

    Mocha_UnmountFS("slccmpt01");
    Mocha_DeInitLibrary();
}

void restoreSysconfIfNeeded()
{
    bool restoreCheck = false;
    WUPSStorageAPI::Get("restoreSysconf", restoreCheck);
    if (!restoreCheck)
        return;

    std::error_code ec;
    auto fileSize = std::filesystem::file_size(SYSCONF_SD_PATH, ec);
    if (ec) {
        DEBUG_FUNCTION_LINE_ERR("File size check failed: %s", ec.message().c_str());
        return;
    }
    if (fileSize != 0x4000) {
        DEBUG_FUNCTION_LINE_ERR("SD's SYSCONF is wrong size, should be 16,384 bytes");
        return;
    }

    if (initMocha() != MOCHA_RESULT_SUCCESS)
        return;

    if (MountWrapper("slccmpt01", "/dev/slccmpt01", "/vol/storage_slccmpt01") != MOCHA_RESULT_SUCCESS) {
        Mocha_DeInitLibrary();
        return;
    }

    try {
        if (std::filesystem::copy_file(SYSCONF_SD_PATH,
                                       SYSCONF_NAND_PATH,
                                       std::filesystem::copy_options::overwrite_existing,
                                       ec)) {
            WUPSStorageAPI::Store("restoreSysconf", false);
            WUPSStorageAPI::SaveStorage();
            std::filesystem::remove(SYSCONF_SD_PATH, ec);
        }
        if (ec) {
            DEBUG_FUNCTION_LINE_ERR("Copy or remove failed: %s", ec.message().c_str());
        }
    } catch (const std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Copy file exception: %s", e.what());
    }

    Mocha_UnmountFS("slccmpt01");
    Mocha_DeInitLibrary();
}
