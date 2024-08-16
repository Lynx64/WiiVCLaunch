#include "main.h"
#include "config.h"
#include "globals.hpp"
#include "logger.h"
#include "notifications.h"
#include "sysconf_preserver.h"
#include <avm/tv.h>
#include <coreinit/dynload.h>
#include <coreinit/mcp.h>
#include <coreinit/thread.h>
#include <coreinit/title.h>
#include <nn/acp/title.h>
#include <nn/cmpt/cmpt.h>
#include <nn/erreula.h>
#include <notifications/notifications.h>
#include <proc_ui/procui.h>
#include <wups.h>

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
    restoreSysconf();
}

DEINITIALIZE_PLUGIN()
{
    NotificationModule_DeInitLibrary();
}

extern "C" int32_t CMPTAcctSetDrcCtrlEnabled(int32_t enable);

static OSDynLoad_Module erreulaModule                                               = nullptr;
//ErrEula functions copied from <erreula/rpl_interface.h> in wut
static void (*dyn_ErrEulaAppearError)(const nn::erreula::AppearArg &arg)            = nullptr;
static void (*dyn_ErrEulaCalc)(const nn::erreula::ControllerInfo &controllerInfo)   = nullptr;
static void (*dyn_ErrEulaDisappearError)()                                          = nullptr;
static nn::erreula::State (*dyn_ErrEulaGetStateErrorViewer)()                       = nullptr;
static bool (*dyn_ErrEulaIsDecideSelectButtonError)()                               = nullptr;
static bool (*dyn_ErrEulaIsDecideSelectLeftButtonError)()                           = nullptr;
static bool (*dyn_ErrEulaIsDecideSelectRightButtonError)()                          = nullptr;

static bool sLaunchingWiiGame = false;
static bool sInputRedirectionActive = false;

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

ON_APPLICATION_START()
{
    if (OSGetTitleID() == 0x0005001010040000 || // Wii U Menu JPN
        OSGetTitleID() == 0x0005001010040100 || // Wii U Menu USA
        OSGetTitleID() == 0x0005001010040200) { // Wii U Menu EUR
        gInWiiUMenu = true;
        sLaunchingWiiGame = false;
    } else {
        gInWiiUMenu = false;
    }
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

    if (sLaunchingWiiGame || !gUseCustomDialogs || !gInWiiUMenu) {
        //sLaunchingWiiGame: the rest of this function has already ran once, no need to run again (ACPGetLaunchMetaXml can get called twice)
        return result;
    }
    if (result != ACP_RESULT_SUCCESS) {
        sLaunchingWiiGame = false;
        return result;
    }
    sLaunchingWiiGame = true;

    //check if wii game launched, if not then return
    MCPTitleListType titleInfo;
    int32_t mcpHandle = MCP_Open();
    MCPError mcpError = MCP_GetTitleInfo(mcpHandle, metaXml->title_id, &titleInfo);
    MCP_Close(mcpHandle);
    if (mcpError != 0 || titleInfo.appType != MCP_APP_TYPE_GAME_WII) {
        return result;
    }

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
            return result;
        } else if (!DRC_USE && gAutolaunchNoDRCSupport != DISPLAY_OPTION_CHOOSE) {
            setDisplay(gAutolaunchNoDRCSupport);
            if (gNotificationTheme != NOTIFICATION_THEME_OFF)
                showAutolaunchNotification(gAutolaunchNoDRCSupport);
            return result;
        }
    } else {
        activateCursor = false;
    }

    //load erreula
    if (OSDynLoad_Acquire("erreula.rpl", &erreulaModule) != OS_DYNLOAD_OK) {
        return result;
    }
    if (OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaAppearError__3RplFRCQ3_2nn7erreula9AppearArg", (void**) &dyn_ErrEulaAppearError) != OS_DYNLOAD_OK ||
        OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaCalc__3RplFRCQ3_2nn7erreula14ControllerInfo", (void**) &dyn_ErrEulaCalc) != OS_DYNLOAD_OK ||
        OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaDisappearError__3RplFv", (void**) &dyn_ErrEulaDisappearError) != OS_DYNLOAD_OK ||
        OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaGetStateErrorViewer__3RplFv", (void**) &dyn_ErrEulaGetStateErrorViewer) != OS_DYNLOAD_OK ||
        OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaIsDecideSelectButtonError__3RplFv", (void**) &dyn_ErrEulaIsDecideSelectButtonError) != OS_DYNLOAD_OK ||
        OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaIsDecideSelectLeftButtonError__3RplFv", (void**) &dyn_ErrEulaIsDecideSelectLeftButtonError) != OS_DYNLOAD_OK ||
        OSDynLoad_FindExport(erreulaModule, OS_DYNLOAD_EXPORT_FUNC, "ErrEulaIsDecideSelectRightButtonError__3RplFv", (void**) &dyn_ErrEulaIsDecideSelectRightButtonError) != OS_DYNLOAD_OK) {
        
        OSDynLoad_Release(erreulaModule);
        return result;
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
                appearArg.errorArg.errorMessage = u"\n\nSelect a display option.\n\n\n\ue07d More options";
            } else {
                appearArg.errorArg.errorMessage = u"\n\nSelect a display option.\n\n\n\ue07d Refresh";
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
        }
        
    } //end while

    dyn_ErrEulaDisappearError();
    OSDynLoad_Release(erreulaModule);

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

    return result;
}

DECL_FUNCTION(int32_t, CMPTExPrepareLaunch, uint32_t unk1, void *unk2, uint32_t unk3)
{
    setResolution(gSetResolution);
    backupSysconf();
    return real_CMPTExPrepareLaunch(unk1, unk2, unk3);
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

//replace only for Wii U Menu process
WUPS_MUST_REPLACE_FOR_PROCESS(MCP_TitleList, WUPS_LOADER_LIBRARY_COREINIT, MCP_TitleList, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(ACPGetLaunchMetaXml, WUPS_LOADER_LIBRARY_NN_ACP, ACPGetLaunchMetaXml, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(CMPTLaunchMenu, WUPS_LOADER_LIBRARY_NN_CMPT, CMPTLaunchMenu, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(CMPTLaunchDataManager, WUPS_LOADER_LIBRARY_NN_CMPT, CMPTLaunchDataManager, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(CMPTAcctSetDrcCtrlEnabled, WUPS_LOADER_LIBRARY_NN_CMPT, CMPTAcctSetDrcCtrlEnabled, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(WPADProbe, WUPS_LOADER_LIBRARY_PADSCORE, WPADProbe, WUPS_FP_TARGET_PROCESS_WII_U_MENU);

WUPS_MUST_REPLACE_FOR_PROCESS(CMPTExPrepareLaunch, WUPS_LOADER_LIBRARY_NN_CMPT, CMPTExPrepareLaunch, WUPS_FP_TARGET_PROCESS_GAME);
