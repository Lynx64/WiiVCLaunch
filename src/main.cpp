#include "main.h"
#include "config.h"
#include "globals.hpp"
#include "logger.h"
#include <wups.h>
#include <coreinit/mcp.h>
#include <nn/acp/title.h>
#include <coreinit/dynload.h>
#include <nn/erreula.h>
#include <coreinit/thread.h>
#include <nn/cmpt/cmpt.h>
#include <proc_ui/procui.h>

// Mandatory plugin info
WUPS_PLUGIN_NAME("Wii VC Launch");
WUPS_PLUGIN_DESCRIPTION(DESCRIPTION);
WUPS_PLUGIN_VERSION(VERSION);
WUPS_PLUGIN_AUTHOR("Lynx64");
WUPS_PLUGIN_LICENSE("GPLv3");

WUPS_USE_WUT_DEVOPTAB();

// Called when exiting the plugin loader
INITIALIZE_PLUGIN()
{
    initConfig();
}

extern "C" void AVMGetCurrentPort(int32_t *port);
extern "C" void AVMSetTVScanResolution(int32_t res);
extern "C" int32_t CMPTAcctSetDrcCtrlEnabled(int32_t);

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
}

static const char16_t * displayOptionToString(int32_t displayOption)
{
    switch (displayOption)
    {
    case DISPLAY_OPTION_USE_DRC:
        return u"Use \ue087 as controller";
    case DISPLAY_OPTION_TV:
        return u"TV Only";
    case DISPLAY_OPTION_BOTH:
        return u"TV and \ue087";
    case DISPLAY_OPTION_DRC:
        return u"\ue087 only";
    default:
        return u"";
    }
}

ON_APPLICATION_START()
{
    if (sLaunchingWiiGame) {
        sLaunchingWiiGame = false;
        //set resolution
        //adapted from https://github.com/WiiDatabase/Boot2vWii/blob/a7c18a768f5b2183cd94e42f2af81a90f13b8eb1/source/main.c#LL30C10-L30C10
        if (gSetResolution != SET_RESOLUTION_NONE) {
            int32_t outPort = 0;
            AVMGetCurrentPort(&outPort);

            if (outPort > 3) {
                outPort = 0;
            }
            if (outPort < 2) {
                AVMSetTVScanResolution(gSetResolution);
            }
        }
    }
}

//patch the app type of Wii games to a Wii U game on the Wii U Menu. This avoids the built in Wii dialogs
DECL_FUNCTION(int32_t, MCP_TitleList, int32_t handle, uint32_t *outTitleCount, MCPTitleListType *titleList, uint32_t size)
{
    int32_t result = real_MCP_TitleList(handle, outTitleCount, titleList, size);

    if (gUseCustomDialogs) {
        uint32_t titleCount = *outTitleCount;
        for (uint32_t i = 0; i < titleCount; i++) {
            if (titleList[i].appType == MCP_APP_TYPE_GAME_WII) titleList[i].appType = MCP_APP_TYPE_GAME;
        }
    }

    return result;
}

DECL_FUNCTION(int32_t, ACPGetLaunchMetaXml, ACPMetaXml *metaXml)
{
    int32_t result = real_ACPGetLaunchMetaXml(metaXml);
    if (result != ACP_RESULT_SUCCESS) {
        sLaunchingWiiGame = false;
        return result;
    }

    //check if wii game launched, if not then set launchingwiigame to false and return
    MCPTitleListType titleInfo;
    int32_t mcpHandle = MCP_Open();
    MCPError mcpError = MCP_GetTitleInfo(mcpHandle, metaXml->title_id, &titleInfo);
    MCP_Close(mcpHandle);
    if (mcpError != 0 || titleInfo.appType != MCP_APP_TYPE_GAME_WII) {
        sLaunchingWiiGame = false;
        return result;
    }

    if (sLaunchingWiiGame) {
        //the rest of this function has already ran once, no need to run again (ACPGetLaunchMetaXml can get called twice)
        return result;
    }
    sLaunchingWiiGame = true;

    if (!gUseCustomDialogs) return result;

    //check if A button held, if so then skip autolaunch check
    VPADStatus vpadStatus;
    VPADReadError vpadError;
    KPADStatus kpadStatus[4];
    KPADError kpadError[4];
    uint32_t buttonsHeld = 0;

    VPADRead(VPAD_CHAN_0, &vpadStatus, 1, &vpadError);
    if (vpadError == VPAD_READ_SUCCESS) {
        buttonsHeld = vpadStatus.hold;
    }

    for (int32_t i = 0; i < 4; i++) {
        if (KPADReadEx((KPADChan) i, &kpadStatus[i], 1, &kpadError[i]) > 0) {
            if (kpadError[i] == KPAD_ERROR_OK && kpadStatus[i].extensionType != 0xFF) {
                if (kpadStatus[i].extensionType == WPAD_EXT_CORE || kpadStatus[i].extensionType == WPAD_EXT_NUNCHUK) {
                    buttonsHeld |= remapWiiMoteButtons(kpadStatus[i].hold);
                } else {
                    buttonsHeld |= remapClassicButtons(kpadStatus[i].classic.hold);
                }
            }
        }
    }

    if (!(buttonsHeld & VPAD_BUTTON_A)) {
        //check autolaunch
        if (metaXml->drc_use == 65537 && gAutolaunchDRCSupported != DISPLAY_OPTION_CHOOSE) {
            if (gAutolaunchDRCSupported == DISPLAY_OPTION_USE_DRC) {
                CMPTAcctSetScreenType(CMPT_SCREEN_TYPE_BOTH);
                CMPTAcctSetDrcCtrlEnabled(1);
            } else {
                CMPTAcctSetDrcCtrlEnabled(0);
                CMPTAcctSetScreenType((CmptScreenType) gAutolaunchDRCSupported);
            }

            if (CMPTCheckScreenState() < 0)
                CMPTAcctSetScreenType(CMPT_SCREEN_TYPE_DRC);

            return result;
        } else if (metaXml->drc_use != 65537 && gAutolaunchNoDRCSupport != DISPLAY_OPTION_CHOOSE) {
            CMPTAcctSetDrcCtrlEnabled(0);
            CMPTAcctSetScreenType((CmptScreenType) gAutolaunchNoDRCSupport);

            if (CMPTCheckScreenState() < 0)
                CMPTAcctSetScreenType(CMPT_SCREEN_TYPE_DRC);

            return result;
        }
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

    //read recent order
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        OSDynLoad_Release(erreulaModule);
        return result;
    }
    
    int32_t recent[4];
    if (WUPS_GetInt(nullptr, "recent0", &recent[0]) != WUPS_STORAGE_ERROR_SUCCESS ||
        WUPS_GetInt(nullptr, "recent1", &recent[1]) != WUPS_STORAGE_ERROR_SUCCESS ||
        WUPS_GetInt(nullptr, "recent2", &recent[2]) != WUPS_STORAGE_ERROR_SUCCESS ||
        WUPS_GetInt(nullptr, "recent3", &recent[3]) != WUPS_STORAGE_ERROR_SUCCESS) {
        
        //failed to read values from storage - use default values
        recent[0] = DISPLAY_OPTION_USE_DRC;
        recent[1] = DISPLAY_OPTION_TV;
        recent[2] = DISPLAY_OPTION_BOTH;
        recent[3] = DISPLAY_OPTION_DRC;
    }

    WUPS_CloseStorage();

    //map display options to erreula button positions
    int32_t position[4];
    if (gDisplayOptionsOrder == DISPLAY_OPTIONS_ORDER_RECENT) {
        int32_t positionI = 0;
        for (int32_t i = 0; i < 4; i++) {
            if (recent[i] == DISPLAY_OPTION_USE_DRC && metaXml->drc_use != 65537) {
                continue;
            }
            position[positionI] = recent[i];
            positionI++;
        }

        if (metaXml->drc_use != 65537)
            position[3] = position[2];
    } else { //DISPLAY_OPTIONS_ORDER_DEFAULT
        if (metaXml->drc_use == 65537) {
            position[0] = DISPLAY_OPTION_USE_DRC;
            position[1] = DISPLAY_OPTION_TV;
            position[2] = DISPLAY_OPTION_BOTH;
            position[3] = DISPLAY_OPTION_DRC;
        } else {
            position[0] = DISPLAY_OPTION_TV;
            position[1] = DISPLAY_OPTION_BOTH;
            position[2] = DISPLAY_OPTION_DRC;
            position[3] = position[2];
        }
    }

    //set the values for the error viewer that we will keep the same
    nn::erreula::AppearArg appearArg;
    appearArg.errorArg.renderTarget = nn::erreula::RenderTarget::Both;
    appearArg.errorArg.controllerType = nn::erreula::ControllerType::DrcGamepad;
    appearArg.errorArg.errorMessage = u"\n\nSelect a display option.\n\n\n\ue07d More options";

    bool redraw = true;
    bool activateCursor = true;
    bool onFirstPage = true;
    int32_t selectedDisplay = position[0];

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
            activateCursor = true;

            if (onFirstPage) {
                appearArg.errorArg.button1Label = displayOptionToString(position[0]);
                appearArg.errorArg.button2Label = displayOptionToString(position[1]);
                appearArg.errorArg.errorType = nn::erreula::ErrorType::Message2Button;
            } else if (metaXml->drc_use == 65537) {
                appearArg.errorArg.button1Label = displayOptionToString(position[2]);
                appearArg.errorArg.button2Label = displayOptionToString(position[3]);
                appearArg.errorArg.errorType = nn::erreula::ErrorType::Message2Button;
            } else {
                appearArg.errorArg.button1Label = displayOptionToString(position[2]);
                appearArg.errorArg.errorType = nn::erreula::ErrorType::Message1Button;
            }
            dyn_ErrEulaAppearError(appearArg);
            continue;
        }

        if (dyn_ErrEulaIsDecideSelectLeftButtonError()) {
            selectedDisplay = onFirstPage ? position[0] : position[2];
            break;
        } else if (dyn_ErrEulaIsDecideSelectRightButtonError()) {
            //note when using Message1Button, IsDecideSelectRight returns true, IsDecideSelectLeft returns false
            selectedDisplay = onFirstPage ? position[1] : position[3];
            break;
        }

        buttonsHeld = 0;

        VPADRead(VPAD_CHAN_0, &vpadStatus, 1, &vpadError);
        if (vpadError == VPAD_READ_SUCCESS) {
            buttonsHeld = vpadStatus.hold;
        }

        for (int32_t i = 0; i < 4; i++) {
            if (KPADReadEx((KPADChan) i, &kpadStatus[i], 1, &kpadError[i]) > 0) {
                if (kpadError[i] == KPAD_ERROR_OK && kpadStatus[i].extensionType != 0xFF) {
                    if (kpadStatus[i].extensionType == WPAD_EXT_CORE || kpadStatus[i].extensionType == WPAD_EXT_NUNCHUK) {
                        buttonsHeld |= remapWiiMoteButtons(kpadStatus[i].hold);
                    } else {
                        buttonsHeld |= remapClassicButtons(kpadStatus[i].classic.hold);
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
            onFirstPage = !onFirstPage;
            dyn_ErrEulaDisappearError();
        }
        
    } //end while

    dyn_ErrEulaDisappearError();
    OSDynLoad_Release(erreulaModule);

    if (selectedDisplay == DISPLAY_OPTION_USE_DRC) {
        CMPTAcctSetScreenType(CMPT_SCREEN_TYPE_BOTH);
        CMPTAcctSetDrcCtrlEnabled(1);
    } else {
        CMPTAcctSetDrcCtrlEnabled(0);
        CMPTAcctSetScreenType((CmptScreenType) selectedDisplay);
    }

    if (CMPTCheckScreenState() < 0)
        CMPTAcctSetScreenType(CMPT_SCREEN_TYPE_DRC);

    //check if we need to update recent order
    if (selectedDisplay != recent[0] && WUPS_OpenStorage() == WUPS_STORAGE_ERROR_SUCCESS) {
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
        WUPS_StoreInt(nullptr, "recent0", recent[0]);
        WUPS_StoreInt(nullptr, "recent1", recent[1]);
        WUPS_StoreInt(nullptr, "recent2", recent[2]);
        WUPS_StoreInt(nullptr, "recent3", recent[3]);

        WUPS_CloseStorage();
    }

    return result;
}

//replace only for Wii U Menu
WUPS_MUST_REPLACE_FOR_PROCESS(MCP_TitleList, WUPS_LOADER_LIBRARY_COREINIT, MCP_TitleList, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
WUPS_MUST_REPLACE_FOR_PROCESS(ACPGetLaunchMetaXml, WUPS_LOADER_LIBRARY_NN_ACP, ACPGetLaunchMetaXml, WUPS_FP_TARGET_PROCESS_WII_U_MENU);
