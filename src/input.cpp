#include "input.h"

#include <padscore/kpad.h>
#include <vpad/input.h>

// remap buttons functions copied from https://github.com/wiiu-env/WiiUPluginLoaderBackend/blob/cb527add76c95bff3fb1ddef7a016fec3db4c497/source/utils/ConfigUtils.cpp#LL35C7-L35C7
uint32_t remapWiiMoteButtons(uint32_t buttons)
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

uint32_t remapClassicButtons(uint32_t buttons)
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
// end of copied functions

// Reads input from both VPAD (GamePad) and KPAD (Wii Remotes), remapping button presses to VPAD format and combining them into a single bitmask.
uint32_t readCombinedInput(VPADStatus &vpadStatus, VPADReadError &vpadError, KPADStatus kpadStatus[4], KPADError kpadError[4])
{
    uint32_t buttonsHeld = 0;

    if (VPADRead(VPAD_CHAN_0, &vpadStatus, 1, &vpadError) > 0 && vpadError == VPAD_READ_SUCCESS) {
        buttonsHeld = vpadStatus.hold;
    }

    for (int32_t chan = 0; chan < 4; chan++) {
        if (KPADReadEx((KPADChan)chan, &kpadStatus[chan], 1, &kpadError[chan]) > 0) {
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

    return buttonsHeld;
}
