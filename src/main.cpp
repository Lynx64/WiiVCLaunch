#include "main.h"
#include "config.h"
#include "globals.hpp"
#include "logger.h"
#include "mocha.h"
#include "notifications.h"
#include "sysconf_preserver.h"

#include <avm/tv.h>
#include <coreinit/dynload.h>
#include <coreinit/mcp.h>
#include <coreinit/memorymap.h>
#include <coreinit/thread.h>
#include <coreinit/title.h>
#include <nn/acp/title.h>
#include <nn/cmpt/cmpt.h>
#include <nn/erreula.h>
#include <proc_ui/procui.h>

#include <wups.h>

#include <function_patcher/function_patching.h>
#include <notifications/notifications.h>

using namespace std::literals;

// Mandatory plugin info
WUPS_PLUGIN_NAME("Wii VC Launch");
WUPS_PLUGIN_DESCRIPTION(DESCRIPTION);
WUPS_PLUGIN_VERSION(VERSION);
WUPS_PLUGIN_AUTHOR("Lynx64");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();

// Gets called ONCE when the plugin was loaded
INITIALIZE_PLUGIN()
{
    initConfig();
    initNotifications();
    restoreSysconfIfNeeded();
    FunctionPatcher_InitLibrary();
}

DEINITIALIZE_PLUGIN()
{
    NotificationModule_DeInitLibrary();
    FunctionPatcher_DeInitLibrary();
}

extern "C" int32_t CMPTAcctSetDrcCtrlEnabled(int32_t enable);

static OSDynLoad_Module erreulaModule                                               = nullptr;
//ErrEula functions copied from <erreula/rpl_interface.h> in wut
static void (*dyn_ErrEulaAppearError)(const nn::erreula::AppearArg &arg)            = nullptr;
static void (*dyn_ErrEulaCalc)(const nn::erreula::ControllerInfo &controllerInfo)   = nullptr;
static void (*dyn_ErrEulaDisappearError)()                                          = nullptr;
static nn::erreula::State (*dyn_ErrEulaGetStateErrorViewer)()                       = nullptr;
static bool (*dyn_ErrEulaIsDecideSelectLeftButtonError)()                           = nullptr;
static bool (*dyn_ErrEulaIsDecideSelectRightButtonError)()                          = nullptr;

static bool sLaunchingWiiGame = false;
static bool sInputRedirectionActive = false;
static bool sUserCancelledCustomDialogs = false;

//remap buttons functions copied from https://github.com/wiiu-env/WiiUPluginLoaderBackend/blob/cb527add76c95bff3fb1ddef7a016fec3db4c497/source/utils/ConfigUtils.cpp#LL35C7-L35C7
static uint32_t remapWiiMoteButtons(uint32_t buttons)
{
    uint32_t convButtons = 0;

    if (buttons & WPAD_BUTTON_LEFT)
        convButtons |= VPAD_BUTTON_LEFT;

    if (buttons & WPAD_BUTTON_RIGHT)
        convButtons |= VPAD_BUTTON_RIGHT;

    if (buttons & WPAD_BUTTON_DOWN)
        convButtons |= VPAD_BUTTON_DOWN;

    if (buttons & WPAD_BUTTON_UP)
        convButtons |= VPAD_BUTTON_UP;

    if (buttons & WPAD_BUTTON_PLUS)
        convButtons |= VPAD_BUTTON_PLUS;

    if (buttons & WPAD_BUTTON_B)
        convButtons |= VPAD_BUTTON_B;

    if (buttons & WPAD_BUTTON_A)
        convButtons |= VPAD_BUTTON_A;

    if (buttons & WPAD_BUTTON_MINUS)
        convButtons |= VPAD_BUTTON_MINUS;

    if (buttons & WPAD_BUTTON_HOME)
        convButtons |= VPAD_BUTTON_HOME;

    return convButtons;
}

static uint32_t remapClassicButtons(uint32_t buttons)
{
    uint32_t convButtons = 0;

    if (buttons & WPAD_CLASSIC_BUTTON_LEFT)
        convButtons |= VPAD_BUTTON_LEFT;

    if (buttons & WPAD_CLASSIC_BUTTON_RIGHT)
        convButtons |= VPAD_BUTTON_RIGHT;

    if (buttons & WPAD_CLASSIC_BUTTON_DOWN)
        convButtons |= VPAD_BUTTON_DOWN;

    if (buttons & WPAD_CLASSIC_BUTTON_UP)
        convButtons |= VPAD_BUTTON_UP;

    if (buttons & WPAD_CLASSIC_BUTTON_PLUS)
        convButtons |= VPAD_BUTTON_PLUS;

    if (buttons & WPAD_CLASSIC_BUTTON_X)
        convButtons |= VPAD_BUTTON_X;

    if (buttons & WPAD_CLASSIC_BUTTON_Y)
        convButtons |= VPAD_BUTTON_Y;

    if (buttons & WPAD_CLASSIC_BUTTON_B)
        convButtons |= VPAD_BUTTON_B;

    if (buttons & WPAD_CLASSIC_BUTTON_A)
        convButtons |= VPAD_BUTTON_A;

    if (buttons & WPAD_CLASSIC_BUTTON_MINUS)
        convButtons |= VPAD_BUTTON_MINUS;

    if (buttons & WPAD_CLASSIC_BUTTON_HOME)
        convButtons |= VPAD_BUTTON_HOME;

    if (buttons & WPAD_CLASSIC_BUTTON_ZR)
        convButtons |= VPAD_BUTTON_ZR;

    if (buttons & WPAD_CLASSIC_BUTTON_ZL)
        convButtons |= VPAD_BUTTON_ZL;

    if (buttons & WPAD_CLASSIC_BUTTON_R)
        convButtons |= VPAD_BUTTON_R;

    if (buttons & WPAD_CLASSIC_BUTTON_L)
        convButtons |= VPAD_BUTTON_L;

    return convButtons;
} //end of copied functions

static const char * displayOptionToStringWithoutIcons(int32_t displayOption)
{
    // the GamePad icon doesn't look good on the notification's small font size
    switch (displayOption)
    {
    case DISPLAY_OPTION_USE_DRC:
        return "Use GamePad as controller";
    case DISPLAY_OPTION_TV:
        return "TV Only";
    case DISPLAY_OPTION_BOTH:
        return "TV and GamePad";
    case DISPLAY_OPTION_DRC:
        return "GamePad screen only";
    default:
        return "";
    }
}

static const char16_t * displayOptionToString16(int32_t displayOption)
{
    switch (displayOption)
    {
    case DISPLAY_OPTION_USE_DRC:
        return u"Use \uE087 as controller";
    case DISPLAY_OPTION_TV:
        return u"TV Only";
    case DISPLAY_OPTION_BOTH:
        return u"TV and \uE087";
    case DISPLAY_OPTION_DRC:
        return u"\uE087 screen only";
    default:
        return u"";
    }
}

static void showAutolaunchNotification(int32_t displayOption)
{
    char text[41];
    snprintf(text, sizeof(text), "Autolaunching: %s", displayOptionToStringWithoutIcons(displayOption));
    NotificationModule_AddInfoNotification(text);
}

static void setResolution(int32_t resolution)
{
    if (resolution == SET_RESOLUTION_NONE) return;

    if (resolution == SET_RESOLUTION_576I || resolution == SET_RESOLUTION_576I_43) {
        AVMSetTVVideoRegion(AVM_TV_VIDEO_REGION_PAL, TVEGetCurrentPort(), AVM_TV_RESOLUTION_576I);
    } else {
        if (resolution > SET_RESOLUTION_4_3_MODIFIER) {
            AVMSetTVScanResolution((AVMTvResolution) (resolution - SET_RESOLUTION_4_3_MODIFIER));
        } else {
            AVMSetTVScanResolution((AVMTvResolution) resolution);
        }
    }

    if (resolution > SET_RESOLUTION_4_3_MODIFIER)
        AVMSetTVAspectRatio(AVM_TV_ASPECT_RATIO_4_3);
}

static void setDisplay(int32_t displayOption)
{
    if (displayOption == DISPLAY_OPTION_USE_DRC) {
        CMPTAcctSetScreenType(CMPT_SCREEN_TYPE_BOTH);
        CMPTAcctSetDrcCtrlEnabled(1);
    } else {
        CMPTAcctSetDrcCtrlEnabled(0);
        CMPTAcctSetScreenType((CmptScreenType) displayOption);
    }

    if (CMPTCheckScreenState() < 0)
        CMPTAcctSetScreenType(CMPT_SCREEN_TYPE_DRC);
}

// used with lib Function Patcher
DECL_FUNCTION(int32_t, nn_cmpt_FUN_02002A88, int32_t mcpHandle, uint32_t outputType, int32_t wants576i, uint32_t param4, uint32_t param5)
{
    wants576i = 0;
    return real_nn_cmpt_FUN_02002A88(mcpHandle, outputType, wants576i, param4, param5);
}

static PatchedFunctionHandle sPatchedFunctionHandle1 = 0;

static PatchedFunctionHandle addFunctionPatch_nn_cmpt_FUN_02002A88(uint32_t address)
{
    uint32_t physicalAddress = OSEffectiveToPhysical(address);
    DEBUG_FUNCTION_LINE_INFO("effective %08X, physical %08X", address, physicalAddress);

    PatchedFunctionHandle patchedFunctionHandle = 0;
    function_replacement_data_t patchData REPLACE_FUNCTION_VIA_ADDRESS_FOR_PROCESS(nn_cmpt_FUN_02002A88, physicalAddress, address, FP_TARGET_PROCESS_GAME_AND_MENU);
    FunctionPatcherStatus result = FunctionPatcher_AddFunctionPatch(&patchData, &patchedFunctionHandle, nullptr);

    if (result != FUNCTION_PATCHER_RESULT_SUCCESS) {
        DEBUG_FUNCTION_LINE_ERR("AddFunctionPatch returned %d", result);
        return 0;
    }

    return patchedFunctionHandle;
}

static void setupPatches_nn_cmpt()
{
    OSDynLoad_Module cmptModule = nullptr;
    if ((OSDynLoad_IsModuleLoaded("nn_cmpt", &cmptModule) != OS_DYNLOAD_OK) || !cmptModule) {
        DEBUG_FUNCTION_LINE("nn_cmpt not loaded");
        return;
    }

    uint32_t targetFunctionAddress = 0;
    // the closest function symbol before the function we want to patch (.text + 2548h)
    if (OSDynLoad_FindExport(cmptModule, OS_DYNLOAD_EXPORT_FUNC, "CMPTAcctClearInternalState", (void **) &targetFunctionAddress) != OS_DYNLOAD_OK ||
        targetFunctionAddress == 0) {
        DEBUG_FUNCTION_LINE_ERR("Failed to find export");
        return;
    }

    // shortcut to get target address
    // .text + 2548h -> .text + 2A88h
    targetFunctionAddress += 0x2A88 - 0x2548;
    sPatchedFunctionHandle1 = addFunctionPatch_nn_cmpt_FUN_02002A88(targetFunctionAddress);
}

void newRplLoaded(OSDynLoad_Module module, void *userContext, OSDynLoad_NotifyReason notifyReason, OSDynLoad_NotifyData *rpl)
{
    if (!rpl->name || !std::string_view(rpl->name).ends_with("nn_cmpt.rpl"sv)) {
        return;
    }
    if (notifyReason == OS_DYNLOAD_NOTIFY_LOADED) {
        sPatchedFunctionHandle1 = addFunctionPatch_nn_cmpt_FUN_02002A88(rpl->textAddr + 0x2A88);
    } else { // unloaded
        if (sPatchedFunctionHandle1 != 0) {
            FunctionPatcherStatus removeFunctionPatchResult = FunctionPatcher_RemoveFunctionPatch(sPatchedFunctionHandle1);
            if (removeFunctionPatchResult != FUNCTION_PATCHER_RESULT_SUCCESS) {
                DEBUG_FUNCTION_LINE_ERR("RemoveFunctionPatch returned %d", removeFunctionPatchResult);
            }
            sPatchedFunctionHandle1 = 0;
        }
    }
}

ON_APPLICATION_START()
{
#ifdef DEBUG
    initLogging();
#endif
    if (OSGetTitleID() == 0x0005001010040000 || // Wii U Menu JPN
        OSGetTitleID() == 0x0005001010040100 || // Wii U Menu USA
        OSGetTitleID() == 0x0005001010040200) { // Wii U Menu EUR
        gInWiiUMenu = true;
        sLaunchingWiiGame = false;
    } else {
        gInWiiUMenu = false;
    }

    setupPatches_nn_cmpt();

    OSDynLoad_AddNotifyCallback(&newRplLoaded, nullptr);
}

//patch the app type of Wii games to a Wii U game on the Wii U Menu. This avoids the built in Wii dialogs
DECL_FUNCTION(int32_t, MCP_TitleList, int32_t handle, uint32_t *outTitleCount, MCPTitleListType *titleList, uint32_t size)
{
    int32_t result = real_MCP_TitleList(handle, outTitleCount, titleList, size);

    if (gUseCustomDialogs && gInWiiUMenu) {
        if (result == 0 && outTitleCount && titleList && size != 0) {
            DEBUG_FUNCTION_LINE_INFO("Patching MCP_TitleList in Wii U Menu");
            uint32_t titleCount = *outTitleCount;
            for (uint32_t i = 0; i < titleCount; i++) {
                if (titleList[i].appType == MCP_APP_TYPE_GAME_WII) titleList[i].appType = MCP_APP_TYPE_GAME;
            }
        } else {
            DEBUG_FUNCTION_LINE_WARN("Either real_MCP_TitleList did not return 0 (returned %d)", result);
            DEBUG_FUNCTION_LINE_WARN("or outTitleCount == nullptr or titleList == nullptr");
        }
    }

    return result;
}

DECL_FUNCTION(int32_t, ACPGetLaunchMetaXml, ACPMetaXml *metaXml)
{
    int32_t result = real_ACPGetLaunchMetaXml(metaXml);

    if (!gUseCustomDialogs || !gInWiiUMenu) {
        return result;
    } else if (result != ACP_RESULT_SUCCESS) {
        sLaunchingWiiGame = false;
        return result;
    } else if (sLaunchingWiiGame) {
        //the rest of this function has already ran once, no need to run again (ACPGetLaunchMetaXml can get called twice)
        return ACP_RESULT_SUCCESS;
    }

    //check if wii game launched, if not then return
    MCPTitleListType titleInfo;
    int32_t mcpHandle = MCP_Open();
    MCPError mcpError = MCP_GetTitleInfo(mcpHandle, metaXml->title_id, &titleInfo);
    MCP_Close(mcpHandle);
    if (mcpError != 0 || titleInfo.appType != MCP_APP_TYPE_GAME_WII) {
        return ACP_RESULT_SUCCESS;
    }
    sLaunchingWiiGame = true;

    //read drc_use value
    const bool DRC_USE = metaXml->drc_use == 65537;

    //check if A button held, if so then skip autolaunch check
    VPADStatus vpadStatus{};
    VPADReadError vpadError = VPAD_READ_UNINITIALIZED;
    KPADStatus kpadStatus[4];
    KPADError kpadError[4];
    uint32_t buttonsHeld = 0;
    bool activateCursor = true;

    if (VPADRead(VPAD_CHAN_0, &vpadStatus, 1, &vpadError) > 0 && vpadError == VPAD_READ_SUCCESS) {
        buttonsHeld = vpadStatus.hold;
    }

    for (int32_t chan = 0; chan < 4; chan++) {
        if (KPADReadEx((KPADChan) chan, &kpadStatus[chan], 1, &kpadError[chan]) > 0) {
            if (kpadError[chan] == KPAD_ERROR_OK && kpadStatus[chan].extensionType != 0xFF) {
                if (kpadStatus[chan].extensionType == WPAD_EXT_CORE || kpadStatus[chan].extensionType == WPAD_EXT_NUNCHUK ||
                    kpadStatus[chan].extensionType == WPAD_EXT_MPLUS || kpadStatus[chan].extensionType == WPAD_EXT_MPLUS_NUNCHUK) {
                    buttonsHeld |= remapWiiMoteButtons(kpadStatus[chan].hold);
                } else {
                    buttonsHeld |= remapClassicButtons(kpadStatus[chan].classic.hold);
                }
            }
        }
    }

    if (!(buttonsHeld & VPAD_BUTTON_A)) {
        //check autolaunch
        if (DRC_USE && gAutolaunchDRCSupported != DISPLAY_OPTION_CHOOSE) {
            setDisplay(gAutolaunchDRCSupported);
            if (gNotificationTheme != NOTIFICATION_THEME_OFF)
                showAutolaunchNotification(gAutolaunchDRCSupported);
            return ACP_RESULT_SUCCESS;
        } else if (!DRC_USE && gAutolaunchNoDRCSupport != DISPLAY_OPTION_CHOOSE) {
            setDisplay(gAutolaunchNoDRCSupport);
            if (gNotificationTheme != NOTIFICATION_THEME_OFF)
                showAutolaunchNotification(gAutolaunchNoDRCSupport);
            return ACP_RESULT_SUCCESS;
        }
    } else {
        activateCursor = false;
    }

    //load erreula
    if (OSDynLoad_Acquire("erreula.rpl", &erreulaModule) != OS_DYNLOAD_OK) {
        return ACP_RESULT_SUCCESS;
    }
    if (OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaAppearError__3RplFRCQ3_2nn7erreula9AppearArg", (void**) &dyn_ErrEulaAppearError) != OS_DYNLOAD_OK ||
        OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaCalc__3RplFRCQ3_2nn7erreula14ControllerInfo", (void**) &dyn_ErrEulaCalc) != OS_DYNLOAD_OK ||
        OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaDisappearError__3RplFv", (void**) &dyn_ErrEulaDisappearError) != OS_DYNLOAD_OK ||
        OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaGetStateErrorViewer__3RplFv", (void**) &dyn_ErrEulaGetStateErrorViewer) != OS_DYNLOAD_OK ||
        OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaIsDecideSelectLeftButtonError__3RplFv", (void**) &dyn_ErrEulaIsDecideSelectLeftButtonError) != OS_DYNLOAD_OK ||
        OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaIsDecideSelectRightButtonError__3RplFv", (void**) &dyn_ErrEulaIsDecideSelectRightButtonError) != OS_DYNLOAD_OK) {
        
        OSDynLoad_Release(erreulaModule);
        return ACP_RESULT_SUCCESS;
    }

    int32_t recent[4] = {DISPLAY_OPTION_USE_DRC, DISPLAY_OPTION_TV, DISPLAY_OPTION_BOTH, DISPLAY_OPTION_DRC};
    //read recent order
    if (gDisplayOptionsOrder == DISPLAY_OPTIONS_ORDER_RECENT) {
        if (WUPSStorageAPI::Get("recent0", recent[0]) != WUPS_STORAGE_ERROR_SUCCESS ||
            WUPSStorageAPI::Get("recent1", recent[1]) != WUPS_STORAGE_ERROR_SUCCESS ||
            WUPSStorageAPI::Get("recent2", recent[2]) != WUPS_STORAGE_ERROR_SUCCESS ||
            WUPSStorageAPI::Get("recent3", recent[3]) != WUPS_STORAGE_ERROR_SUCCESS) {
            
            //failed to read values from storage - use default values
            recent[0] = DISPLAY_OPTION_USE_DRC;
            recent[1] = DISPLAY_OPTION_TV;
            recent[2] = DISPLAY_OPTION_BOTH;
            recent[3] = DISPLAY_OPTION_DRC;
        }
    }

    //set the values for the error viewer that we will keep the same
    nn::erreula::AppearArg appearArg;
    appearArg.errorArg.renderTarget = nn::erreula::RenderTarget::Both;
    appearArg.errorArg.controllerType = nn::erreula::ControllerType::DrcGamepad;

    bool redraw = true;
    bool onFirstPage = true;
    int32_t selectedDisplay = recent[0];
    int32_t position[2] = {recent[0], recent[1]};

    while (true) {
        OSSleepTicks(OSMillisecondsToTicks(16));

        if (ProcUIInForeground() == FALSE) {
            sLaunchingWiiGame = false;
            break;
        }

        if (dyn_ErrEulaGetStateErrorViewer() != nn::erreula::State::Visible && dyn_ErrEulaGetStateErrorViewer() != nn::erreula::State::Hidden)
            continue;
        
        if (redraw) {
            redraw = false;

            uint32_t positionI = 0;
            uint32_t skippedOptionsCount = 0;
            bool tvConnected = true; //default to true so tv options are always displayed if non-hdmi is used
            if (TVEGetCurrentPort() == TVE_PORT_HDMI) {
                TVEHdmiState hdmiState = TVE_HDMI_STATE_HTPG_OFF;
                AVMGetHDMIState(&hdmiState);
                if (hdmiState != TVE_HDMI_STATE_DONE && hdmiState != TVE_HDMI_STATE_3RDA)
                    tvConnected = false;
            }
            
            for (uint32_t recentI = 0; recentI < 4; recentI++) {
                if (!DRC_USE && recent[recentI] == DISPLAY_OPTION_USE_DRC)
                    continue;
                if (tvConnected && !onFirstPage && skippedOptionsCount < 2) {
                    skippedOptionsCount++;
                    continue;
                } else if (!tvConnected && (recent[recentI] == DISPLAY_OPTION_TV || recent[recentI] == DISPLAY_OPTION_BOTH)) {
                    continue;
                }
                position[positionI] = recent[recentI];
                positionI++;
                if (positionI > 1)
                    break;
            }

            if (positionI == 0) { //should never happen - filter returned 0 options
                break;
            } else if (positionI == 1) {
                position[1] = position[0];
                appearArg.errorArg.errorType = nn::erreula::ErrorType::Message1Button;
            } else {
                appearArg.errorArg.button2Label = displayOptionToString16(position[1]);
                appearArg.errorArg.errorType = nn::erreula::ErrorType::Message2Button;
            }
            appearArg.errorArg.button1Label = displayOptionToString16(position[0]);
            if (tvConnected) {
                appearArg.errorArg.errorMessage = u"\n\nSelect a display option.\n\n\n\uE07D More options";
            } else {
                appearArg.errorArg.errorMessage = u"\n\nSelect a display option.\n\n\n\uE07D Detect TV";
            }
            dyn_ErrEulaAppearError(appearArg);
            continue;
        }

        if (dyn_ErrEulaIsDecideSelectLeftButtonError()) {
            selectedDisplay = position[0];
            break;
        } else if (dyn_ErrEulaIsDecideSelectRightButtonError()) {
            //note when using Message1Button, IsDecideSelectRight returns true, IsDecideSelectLeft returns false
            selectedDisplay = position[1];
            break;
        }

        buttonsHeld = 0;

        if (VPADRead(VPAD_CHAN_0, &vpadStatus, 1, &vpadError) > 0 && vpadError == VPAD_READ_SUCCESS) {
            buttonsHeld = vpadStatus.hold;
        }

        for (int32_t chan = 0; chan < 4; chan++) {
            if (KPADReadEx((KPADChan) chan, &kpadStatus[chan], 1, &kpadError[chan]) > 0) {
                if (kpadError[chan] == KPAD_ERROR_OK && kpadStatus[chan].extensionType != 0xFF) {
                    if (kpadStatus[chan].extensionType == WPAD_EXT_CORE || kpadStatus[chan].extensionType == WPAD_EXT_NUNCHUK ||
                        kpadStatus[chan].extensionType == WPAD_EXT_MPLUS || kpadStatus[chan].extensionType == WPAD_EXT_MPLUS_NUNCHUK) {
                        buttonsHeld |= remapWiiMoteButtons(kpadStatus[chan].hold);
                    } else {
                        buttonsHeld |= remapClassicButtons(kpadStatus[chan].classic.hold);
                    }
                }
            }
        }

        if (activateCursor) {
            //pass a fake input into Calc to activate the select cursor by default when the dialog appears
            activateCursor = false;
            vpadStatus.hold = VPAD_BUTTON_LEFT;
            kpadStatus[0].hold = WPAD_BUTTON_LEFT;
            kpadStatus[0].classic.hold = WPAD_CLASSIC_BUTTON_LEFT;
            nn::erreula::ControllerInfo controllerInfo;
            controllerInfo.vpad = &vpadStatus;
            controllerInfo.kpad[0] = &kpadStatus[0];
            controllerInfo.kpad[1] = nullptr;
            controllerInfo.kpad[2] = nullptr;
            controllerInfo.kpad[3] = nullptr;
            dyn_ErrEulaCalc(controllerInfo);
        }

        if (buttonsHeld & VPAD_BUTTON_DOWN || buttonsHeld & VPAD_BUTTON_UP) {
            redraw = true;
            activateCursor = true;
            onFirstPage = !onFirstPage;
            dyn_ErrEulaDisappearError();
        } else if (buttonsHeld & VPAD_BUTTON_B) {
            sUserCancelledCustomDialogs = true;
            sLaunchingWiiGame = false;
            break;
        }
        
    } //end while

    if (!sUserCancelledCustomDialogs)
        dyn_ErrEulaDisappearError();
    OSDynLoad_Release(erreulaModule);

    if (sUserCancelledCustomDialogs) {
        return ACP_RESULT_MEDIA_NOT_READY; //return error to abort launching
    } else if (!sLaunchingWiiGame) {
        return ACP_RESULT_SUCCESS; //return early if we're exiting from ProcUI
    }

    setDisplay(selectedDisplay);

    //check if we need to update recent order
    if (gDisplayOptionsOrder == DISPLAY_OPTIONS_ORDER_RECENT && selectedDisplay != recent[0]) {
        //update recent order
        if (selectedDisplay != recent[1]) {
            if (selectedDisplay != recent[2]) {
                recent[3] = recent[2];
            }
            recent[2] = recent[1];
        }
        recent[1] = recent[0];
        recent[0] = selectedDisplay;

        //save new order to storage
        WUPSStorageAPI::Store("recent0", recent[0]);
        WUPSStorageAPI::Store("recent1", recent[1]);
        WUPSStorageAPI::Store("recent2", recent[2]);
        WUPSStorageAPI::Store("recent3", recent[3]);

        WUPSStorageAPI::SaveStorage();
    }

    return ACP_RESULT_SUCCESS;
}

DECL_FUNCTION(bool, ErrEulaIsDecideSelectButtonError__3RplFv)
{
    if (sUserCancelledCustomDialogs) {
        sUserCancelledCustomDialogs = false;
        return true;
    }
    return real_ErrEulaIsDecideSelectButtonError__3RplFv();
}

static void patchNetConfigOverwrite()
{
    if (initMocha() != MOCHA_RESULT_SUCCESS)
        return;
    
    uint32_t data0 = 0;
    uint32_t data1 = 0;
    Mocha_IOSUKernelRead32(0x0503A1C4, &data0); // cmp r6,#0x3 ; bne LAB_0503a178
    Mocha_IOSUKernelRead32(0x0503A1FC, &data1); // adds r5,#0x18 ; add r3,sp,#0x330
    if (data0 != 0x2E03D1D7 || data1 != 0x3518ABCC) {
        Mocha_DeInitLibrary();
        return;
    }

    Mocha_IOSUKernelWrite32(0x0503A1C4, 0x2E02D1D7); // cmp r6,#0x2 ; bne LAB_0503a178
    Mocha_IOSUKernelWrite32(0x0503A1FC, 0x3518ABC6); // adds r5,#0x18 ; add r3,sp,#0x318

    Mocha_DeInitLibrary();
}

DECL_FUNCTION(int32_t, CMPTExPrepareLaunch, uint32_t unk1, void *unk2, uint32_t unk3)
{
    setResolution(gSetResolution);
    if (gPermanentNetConfig)
        patchNetConfigOverwrite();

    int32_t result = real_CMPTExPrepareLaunch(unk1, unk2, unk3);
    
    if (gPreserveSysconf && result == 0)
        backupSysconf();
    return result;
}

static void patchSysconf(uint8_t *confBuffer)
{
    // conf.bin starts at offset 0x80
    if (gSysconfLanguage != SYSCONF_LANGUAGE_NO_OVERRIDE)
        *(confBuffer + 0xA2) = gSysconfLanguage; //language
    //*(confBuffer + 0x9D) = 1; //country
    if (gSysconfEula != SYSCONF_EULA_NO_OVERRIDE)
        *(confBuffer + 0xA4) = gSysconfEula; //eula
}

DECL_FUNCTION(int32_t, CMPTLaunchMenu, void *dataBuffer, uint32_t bufferSize)
{
    setResolution(gWiiMenuSetResolution);
    sInputRedirectionActive = false;
    return real_CMPTLaunchMenu(dataBuffer, bufferSize);
}

DECL_FUNCTION(int32_t, CMPTLaunchDataManager, void *dataBuffer, uint32_t bufferSize)
{
    setResolution(gWiiMenuSetResolution);
    sInputRedirectionActive = false;
    return real_CMPTLaunchDataManager(dataBuffer, bufferSize);
}

DECL_FUNCTION(int32_t, MCP_LaunchCompat, int32_t handle, uint8_t *confBuffer, uint32_t confBufferSize, void *imgsBuffer, uint32_t imgsBufferSize)
{
    if (gPermanentNetConfig)
        patchNetConfigOverwrite();
    
    patchSysconf(confBuffer);

    return real_MCP_LaunchCompat(handle, confBuffer, confBufferSize, imgsBuffer, imgsBufferSize);
}

DECL_FUNCTION(int32_t, CMPTAcctSetDrcCtrlEnabled, int32_t enable)
{
    if (enable == 0 && !sLaunchingWiiGame) {
        int8_t sensorBarEnabled = 0;
        VPADBASEGetSensorBarSetting(VPAD_CHAN_0, &sensorBarEnabled);
        if (!sensorBarEnabled && VPADSetSensorBar(VPAD_CHAN_0, true) == 0) {
            if (gNotificationTheme != NOTIFICATION_THEME_OFF)
                NotificationModule_AddInfoNotification("GamePad sensor bar enabled");
        }
        sInputRedirectionActive = true;
    }
    return real_CMPTAcctSetDrcCtrlEnabled(enable);
}

DECL_FUNCTION(int32_t, WPADProbe, WPADChan chan, WPADExtensionType *outExtensionType)
{
    int32_t result = real_WPADProbe(chan, outExtensionType);
    if (sInputRedirectionActive && result == 0 && outExtensionType && *outExtensionType == WPAD_EXT_PRO_CONTROLLER) {
        *outExtensionType = WPAD_EXT_CLASSIC;
    }
    return result;
}

ON_APPLICATION_REQUESTS_EXIT()
{
    sInputRedirectionActive = false;
}

ON_APPLICATION_ENDS()
{
    if (sPatchedFunctionHandle1 != 0) {
        FunctionPatcherStatus removeFunctionPatchResult = FunctionPatcher_RemoveFunctionPatch(sPatchedFunctionHandle1);
        if (removeFunctionPatchResult != FUNCTION_PATCHER_RESULT_SUCCESS) {
            DEBUG_FUNCTION_LINE_ERR("RemoveFunctionPatch returned %d", removeFunctionPatchResult);
        }
        sPatchedFunctionHandle1 = 0;
    }
#ifdef DEBUG
    deinitLogging();
#endif
}

//replace only for Wii U Menu process
WUPS_MUST_REPLACE_FOR_PROCESS(MCP_TitleList, WUPS_LOADER_LIBRARY_COREINIT, MCP_TitleList, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(ACPGetLaunchMetaXml, WUPS_LOADER_LIBRARY_NN_ACP, ACPGetLaunchMetaXml, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(ErrEulaIsDecideSelectButtonError__3RplFv, WUPS_LOADER_LIBRARY_ERREULA, ErrEulaIsDecideSelectButtonError__3RplFv, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(CMPTLaunchMenu, WUPS_LOADER_LIBRARY_NN_CMPT, CMPTLaunchMenu, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(CMPTLaunchDataManager, WUPS_LOADER_LIBRARY_NN_CMPT, CMPTLaunchDataManager, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(CMPTAcctSetDrcCtrlEnabled, WUPS_LOADER_LIBRARY_NN_CMPT, CMPTAcctSetDrcCtrlEnabled, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(WPADProbe, WUPS_LOADER_LIBRARY_PADSCORE, WPADProbe, WUPS_FP_TARGET_PROCESS_WII_U_MENU);

WUPS_MUST_REPLACE_FOR_PROCESS(CMPTExPrepareLaunch, WUPS_LOADER_LIBRARY_NN_CMPT, CMPTExPrepareLaunch, WUPS_FP_TARGET_PROCESS_GAME);

WUPS_MUST_REPLACE(MCP_LaunchCompat, WUPS_LOADER_LIBRARY_COREINIT, MCP_LaunchCompat);
