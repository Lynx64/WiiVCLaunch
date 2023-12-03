# Wii VC Launch
Wii U plugin for launching Wii VC titles.

## Features
- Select a display option with any controller
- Autolaunch into a specific display option bypassing all dialogs
  - Keep A pressed when launching a game to force open the Select a display option dialogs
- Set resolution to 480p (including 4:3) or 720p (also sets it for Wii Menu) (HDMI only)
- Enable GamePad sensor bar for built in dialogs

Note that it falls back to the GamePad screen if TV not connected.

If custom dialogs are disabled:
- Autolaunch cannot be used
- Set resolution can still be used

## Installation
Download the latest release from the [Releases page](https://github.com/Lynx64/WiiVCLaunch/releases)<br/>
Copy the `.wps` file into `wiiu/environments/[ENVIRONMENT]/plugins`<br/>
where [ENVIRONMENT] is the actual environment name (most likely 'aroma')

## Usage
Open the plugin config menu by pressing L, DPAD Down and Minus on the GamePad, Pro Controller or Classic Controller, or B, DPAD Down and Minus on a Wii Remote.

## Building
For building you need:
- [wups](https://github.com/wiiu-env/WiiUPluginSystem)
- [wut](https://github.com/devkitPro/wut)

then run `make`
