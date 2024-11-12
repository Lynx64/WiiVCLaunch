# Wii VC Launch
Wii U plugin to enhance launching Wii VC titles and Wii Mode.

## Features
- Select a display option (TV Only, TV and GamePad, etc.) with any controller
- Autolaunch into a specific display option bypassing all dialogs
  - Keep A pressed when launching a game to force open the Select a display option dialogs
  - Falls back to the GamePad screen if TV not connected
- Set the resolution to 480p, 720p, 480i, or 576i (including 4:3 variants)
  - Additional separate setting for the Wii Menu resolution
- Enables the GamePad sensor bar for built in dialogs

## Installation
Download the latest release from the [Releases page](https://github.com/Lynx64/WiiVCLaunch/releases/latest)<br/>
Copy the `.wps` file into `wiiu/environments/[ENVIRONMENT]/plugins`<br/>
where [ENVIRONMENT] is the actual environment name (most likely 'aroma')

## Usage
Open the plugin config menu by pressing L, DPAD Down and Minus on the GamePad, Pro Controller or Classic Controller, or B, DPAD Down and Minus on a Wii Remote.

If custom dialogs are disabled:
- Autolaunch cannot be used
- Set resolution can still be used

## Building
For building you need:
- [wups](https://github.com/wiiu-env/WiiUPluginSystem)
- [wut](https://github.com/devkitPro/wut)
- [libnotifications](https://github.com/wiiu-env/libnotifications)
- [libmocha](https://github.com/wiiu-env/libmocha)

then run `make`
