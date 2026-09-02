#pragma once

#include <padscore/kpad.h>
#include <vpad/input.h>

uint32_t remapWiiMoteButtons(uint32_t buttons);
uint32_t remapClassicButtons(uint32_t buttons);

/**
 * Reads the current GamePad and Wii Remote buttons held, remaps them to VPAD button values,
 * and combines them into a single bitmask.
 *
 * @param vpadStatus [out] Receives the latest VPAD status.
 * @param vpadError [out] Receives the VPAD read result.
 * @param kpadStatus [out] Array of 4 KPAD status entries for controllers 0-3, filled by KPADReadEx.
 * @param kpadError [out] Array of 4 KPAD read errors for controllers 0-3.
 * @return A combined button bitmask in VPAD format containing all active buttons.
 */
uint32_t readCombinedInput(VPADStatus &vpadStatus, VPADReadError &vpadError, KPADStatus kpadStatus[4], KPADError kpadError[4]);
