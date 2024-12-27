# Wii VC Launch
Wii U plugin to enhance launching Wii VC titles and Wii Mode.

## Features
### Custom dialogs
- Select a display option (TV Only, TV and GamePad, etc.) with any controller
- Autolaunch into a specific display option bypassing all dialogs
  - Keep A pressed when launching a game to force open the Select a display option dialogs
  - Falls back to the GamePad screen if TV not connected

### Original built in dialogs
- Enables the GamePad sensor bar for built in dialogs
- Allow using a Pro Controller to Select a display option in the built in dialogs

### Video
- Set the resolution to 480p, 720p, 480i, or 576i (including 4:3 variants)
  - Can be set separately for Wii VC and Wii Mode

## Installation
Download the latest release from the [Releases page](https://github.com/Lynx64/WiiVCLaunch/releases/latest) by clicking on `WiiVCLaunch.wps`.<br/>
Copy the `WiiVCLaunch.wps` file into `wiiu/environments/[ENVIRONMENT]/plugins`,<br/>
where [ENVIRONMENT] is the actual environment name (most likely 'aroma').

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

## Building using the Dockerfile
It's possible to use a docker image for building. This way you don't need anything installed on your host system other than Docker.

```
# Build docker image (only needed once)
docker build . -t wiivclaunch-builder

# make
docker run --rm -v ${PWD}:/project wiivclaunch-builder make

# make clean
docker run --rm -v ${PWD}:/project wiivclaunch-builder make clean
```
