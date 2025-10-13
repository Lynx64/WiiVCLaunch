#include "logger.h"
#include "mocha.h"
#include <filesystem>
#include <wups/storage.h>

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

    try {
        bool copySuccess = std::filesystem::copy_file(SYSCONF_NAND_PATH,
                                                      SYSCONF_SD_PATH,
                                                      std::filesystem::copy_options::overwrite_existing);
        if (copySuccess) {
            WUPSStorageAPI::Store("restoreSysconf", true);
            WUPSStorageAPI::SaveStorage();
        }
    } catch (const std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Copy exception: %s", e.what());
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

    try {
        auto fileSize = std::filesystem::file_size(SYSCONF_SD_PATH);
        if (fileSize != 0x4000) {
            DEBUG_FUNCTION_LINE_ERR("SD's SYSCONF is wrong size, should be 16,384 bytes");
            return;
        }
    } catch (const std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("File size exception: %s", e.what());
        return;
    }

    if (initMocha() != MOCHA_RESULT_SUCCESS)
        return;

    if (MountWrapper("slccmpt01", "/dev/slccmpt01", "/vol/storage_slccmpt01") != MOCHA_RESULT_SUCCESS) {
        Mocha_DeInitLibrary();
        return;
    }

    try {
        bool copySuccess = std::filesystem::copy_file(SYSCONF_SD_PATH,
                                                      SYSCONF_NAND_PATH,
                                                      std::filesystem::copy_options::overwrite_existing);
        if (copySuccess) {
            WUPSStorageAPI::Store("restoreSysconf", false);
            WUPSStorageAPI::SaveStorage();
            std::filesystem::remove(SYSCONF_SD_PATH);
        }
    } catch (const std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Copy exception: %s", e.what());
    }

    Mocha_UnmountFS("slccmpt01");
    Mocha_DeInitLibrary();
}
