#include "logger.h"
#include "mocha.h"
#include <filesystem>
#include <wups/storage.h>

void backupSysconf()
{
    if (initMocha() != MOCHA_RESULT_SUCCESS)
        return;

    if (MountWrapper("slccmpt01", "/dev/slccmpt01", "/vol/storage_slccmpt01") != MOCHA_RESULT_SUCCESS) {
        Mocha_DeInitLibrary();
        return;
    }

    bool copySuccess = false;
    try {
        copySuccess = std::filesystem::copy_file("/vol/storage_slccmpt01/shared2/sys/SYSCONF",
                                                 "/vol/external01/wiiu/SYSCONF",
                                                 std::filesystem::copy_options::overwrite_existing);
    } catch (const std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Copy exception: %s", e.what());
    }
    
    Mocha_UnmountFS("slccmpt01");
    Mocha_DeInitLibrary();
    
    if (copySuccess) {
        WUPSStorageAPI::Store("restoreSysconf", true);
        WUPSStorageAPI::SaveStorage();
    }
}

void restoreSysconf()
{
    bool restoreCheck = false;
    WUPSStorageAPI::Get("restoreSysconf", restoreCheck);
    if (!restoreCheck)
        return;
    
    WUPSStorageAPI::Store("restoreSysconf", false);
    WUPSStorageAPI::SaveStorage();
    
    try {
        auto fileSize = std::filesystem::file_size("/vol/external01/wiiu/SYSCONF");
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
        std::filesystem::copy_file("/vol/external01/wiiu/SYSCONF",
                                   "/vol/storage_slccmpt01/shared2/sys/SYSCONF",
                                   std::filesystem::copy_options::overwrite_existing);
        std::filesystem::remove("/vol/external01/wiiu/SYSCONF");
    } catch (const std::exception &e) {
        DEBUG_FUNCTION_LINE_ERR("Copy exception: %s", e.what());
    }
    
    Mocha_UnmountFS("slccmpt01");
    Mocha_DeInitLibrary();
}
