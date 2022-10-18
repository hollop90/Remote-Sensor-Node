# Remote-Sensor-Node
An ultlra-low-power LoRaWAN sensor node based on the MCCI LMIC Library. This repository contiains the source code, 3D CAD, the electronic schematics and some images of the project.

<img src="Images/Sensor%20node%20outdoors.jpg" alt="Sensor node outdoors with solar panel" width="500px"></a>

## Features
- 14.5μA sleep current
- 10 minute transmission interval
- 6km range achieved to a public gateway from ground level

<img src="Images\Project Overview 3.png" alt="Block diagram of project spilt into three main sections. Power, Processing and Networking" width=500></a>

## Code Setup
The project firmware can be compiled and uploaded using either the Arduino IDE or PlatofrmIO.

**N.B. THE CODE WILL NOT RUN UNLESS A COMPATIBLE RADIO IS HOOKED UP CORRECTLY. SEE [SCHEMATIC](./Design%20Files/Electronics%20Design/schematic.pdf) FOR MORE DETAILS** 
### Using Arduino IDE
- Cloning this repository is not strictly necessary in this case as everything can just be copy pasted
- Copy and paste the contents of main.cpp into a new Arduino project
- Install the following libraries. Make sure to install all dependencies when prompted
    ```
    ClosedCube HDC1080 (By ClosedCube)
    LowPower (By  LowPowerLab)
    MCCI LoRaWAN LMIC library (By Terry Moore)
    RV-3028-C7 (By Macro Yau)
    ```
- Locate your Arduino sketchbook. It's path can be found in the Arduino IDE preferences menu
- Navigate to the LMIC project config folder and edit "lmic_project_config.h" to match the example **Project Config** below. 

    (Located in `<SKETCHBOOK_DIR/libraries/MCCI_LoRaWAN_LMIC_library/project_config/lmic_project_config.h`)

    More configuration info can by found in the [LMIC library documentation](https://github.com/mcci-catena/arduino-lmic)
- Select Arduino Pro Mini 8MHz from the tools menu and you are ready to upload
- Open the serial monitor and set the baud rate to 115000 bps

### Using PlatformIO (PIO)
Clone this repository and download the VS Code editor if you don't have it already. Next install the PlatformIO extension. Once complete open this repository in VS Code and follow the steps below
- From VS Code open PIO if not already open
- From the Home tab of PIO click "Open Project"
- Select this repository
- Once open edit this file to match the example **Project Config** 

    (Located in `<REPO_PARENT_DIR>/Remote-Sensor-Node/.pio/libdeps/promini/MCCI LoRaWAN LMIC library/project_config/lmic_project_config.h`)
- Libraries should be automatically installed
- On the bottom of the screen hit the arrow pointing right to upload
- Click on the plug icon to open the serial monitor. Follow the instructions to set the baud rate to 115000 bps

### Project Config

```cpp
// project-specific definitions
#define CFG_eu868 1 // Select the right config for your region
#define CFG_sx1276_radio 1 // Select the right config for your radio
#define DISABLE_PING
#define DISABLE_BEACONS
#define DISABLE_JOIN
#define LMIC_DEBUG_LEVEL 0
#define USE_IDEETRON_AES
```

## Planned for V2

- [ ] Battery monitoring
- Custom PCB
- Ratiation shield for sensors
## Blog
[Solar Powered Sensor](https://ugo-uzoukwu.blogspot.com/)

## Videos
[YouTube Playlist](https://www.youtube.com/playlist?list=PLkDD2GJCGW-Zxzu5pHdPQPp9Yhqgw_unU)

## Live Data
[Tingspeak](https://thingspeak.com/channels/1655776/)