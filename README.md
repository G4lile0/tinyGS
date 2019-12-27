# ESP32-OLED-Fossa-GroundStation for Platformio
The aim of this project is to create an open network of ground stations for the Fossa Satellites distributed all over the world and connected through Internet.

This project is based on ESP32 boards and is compatible with sx126x and sx127x you can build you own board using one of these modules but most of us use a development board like the ones listed in the Supported boards section.

The developers of this project have no relation with the Fossa team in charge of the mission, we are passionate about space and created this project to be able to track and use the satellites as well as supporting the mission.

## [IMPORTANT] Status of the project
Currently we are at an early stage of development (just a couple of weeks of work), the current version is functional but not fully stable and we are actively developing new changes so you will have to update the firmware quite regularly if you want to stay updated.

The first Fossa satellite, FossaSat-1 was launched on December 12, 2019 and it is still in evaluation stage by the Fossa team, so it is important **not to communicate to the satellity and only listen** until the Fossa team says otherwise. 

FossaSat-1 is currently in a healthy state and sending packets, however several issues were found and are being evaluated:
* Satellite antenna and solar panels might not be properly deployed. Received signals from the satellite were too weak compared with theoretical values and simulations which could indicate an improper deployment of the antenna or no deployment at all. The team has tried to command an emergency deployment using high gain antennas during several days with no success.  The dyneema wire holding the solar panels and the anntena is expected to degrade in the orbit atmosphere over the next weeks or months. However it will be highly unlikely to be able to communicate by then as it has been transmitting in short circuit for a long time. Due to this it is not possible to receive FossaSat-1 signals without huge antennas.
* It was discovered a misconfiguration on FossaSat-1 LoRa module. The syncWord parameter used is `0x0F0F` which is not documented and not compatible with sx127x receivers. This means that even with high gain antennas, **it is really difficult to communicate with FossaSat-1 with a sx127x modules** although not impossible as [this was already achieved](https://twitter.com/G4lile0/status/1204311425025486848) with antenna 4 x 23 el (DK7ZB) with tracking, 20 cable, preamp 20 dB, 2 way splitter and module RFM98.
* The satellite might be rotating. The satellite has a Passive Magnetic Stabilization (PMS) system and the Fossa team pointed out that it seems to be stabilizing gradually.

The Fossa team has announced that **two new satellites will be launched on March 2020 and those will be 100% compatible** with all the boards including sx127x, so at this moment the priority is keep improving the project, try to receive communications form FossaSat-1 with high gain antenas and be prepared for the next launch on March when all bords will be compatible. 

We are using Telegram as the mean of communication for the project, there are also two channels where you can suscribe and be updated automátically whenever a new packet is received by the network from the Satellite.
* **Main community chat:** https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q
* **Data channel (station status and received packets):** https://t.me/FOSSASAT_DATA
* **Test channel (simulator packets received by test groundstations):** https://t.me/FOSSASAT_TEST

## Supported boards
* **Heltec WiFi LoRa 32 V1** (433MHz SX1278)
* **Heltec WiFi LoRa 32 V2** (433MHz SX1278) https://heltec.org/project/wifi-lora-32/
* **TTGO LoRa32 V1** (433MHz SX1278)
* **TTGO LoRa32 V2** (433MHz SX1278)

## Supported modules
* sx126x
* sx127x

<p align="center">
<img src="/doc/images/Heltec.jpg" width="300">
</p>

# Quick Install
This project is ready to use with [Platformio](https://platformio.org/). It will take care of all dependencies automatically when building the project. It can also be used with Arduino IDE.

## Platformio (strongly recommended)
Arduino ide instructions bellow.
### Installing platformio
Platformio can be installed as a plugin for many IDEs. You can find a complete list here: https://docs.platformio.org/en/latest/ide.html#desktop-ide

My recommendation is to use VSCode or Atom, you can find installation guides here:

* VSCode: https://docs.platformio.org/en/latest/ide/vscode.html
* Atom: https://docs.platformio.org/en/latest/ide/atom.html

### Open the project in VSCode
Once you have cloned this project to a local directory, you can open it on Visual Studio code in File > Add folder to workspace.

![Add folder to workspace VSCode](/doc/images/add_folder_to_workspace.png "Add folder to workspace VSCode")

Then select the `ESP32-OLED-Fossa-GroundStation` folder inside the repository and click open, make sure it is the root folder and that it has the platformio.ino inside.

![Select folder](/doc/images/Select_folder.png "Select folder")

After that, the project should be loaded in visual studio and ready to configure and build.

### Configure the project
First we need to select the board. To do so, open the `platformio.ini` file and uncomment one of the lines at the beggining of the file depending on the board you are going to use.

```
default_envs = 
; Uncomment by deleting ";" in the line below to select the board
;   heltec_wifi_lora_32
;   ttgo-lora32-v1
;   ttgo-lora32-v2
```

## Build and upload the project
Once the configuration is done, connect the board to the computer and click on the upload button from the platformio toolbar, or go to Terminal -> Run Task -> Upload.

![Upload](/doc/images/upload.png "Upload")

All the dependencies will be downloaded and installed automatically.

Note that if you are a Linux used like me and it is your first time using platformio, you will have to install the udev rules to grant permissions to platformio to upload the program to the board. You can follow the instructions here: https://docs.platformio.org/en/latest/faq.html#platformio-udev-rules

## Arduino IDE
We strongly recommend using platformio, the build process is simpler and it is less likely to have compilation error.

You can install the Arduino IDE by downloading it from [arduino.cc](https://www.arduino.cc/en/Main/Software), we recommend the last version, but you should use v1.6 or above.

### Install the Arduino Core for ESP32
First step is to install support for ESP32 based boards on the Arduino IDE through the Board Manager.

* Start Arduino and open Preferences window.
* Enter `https://dl.espressif.com/dl/package_esp32_index.json` into *Additional Board Manager URLs* field. You can add multiple URLs, separating them with commas. 
* Open Boards Manager from Tools > Board menu and find *esp32* platform.
* Select the version you need from a drop-down box.
* Click *install* button.

### Installing dependencies
This project relies on several third party dependencies that must be installed in order to be able to build the binaries. You can find the dependencies list below.

To install the libraries you can simply copy all files from the GroundStation folder `lib` into your sketchbook\libraries folder. Make sure there are no duplicate libraries that might cause a conflict.

* **RadioLib** (recomended v2.0.1) https://github.com/jgromes/RadioLib
* **ArduinoJson** (recomended v6.13.0 **Required** >v6.0) https://github.com/bblanchon/ArduinoJson
* **ESP8266_SSD1306** (recomended v4.1.0) https://github.com/ThingPulse/esp8266-oled-ssd1306
* **IoTWebConf** (**Required:** 2.3.0@4m1g0) https://github.com/4m1g0/IotWebConf

### Open the project in Arduino IDE
Once you have cloned this project to a local directory, you can open it from the Arduino IDE in `File > Add folder` to workspace. And select the .ino file which is located in `FossaGroundStation > Fossa_GroundStation.ino`

![Open on Arduino IDE](/doc/images/open_arduino.png "Open on Arduino IDE")

### Build and upload the project
The next step is to open the project file ` FossaGroundStation/BoardConfig.h` and uncomment the line matching your board by removing the leading `//`

```
// uncomment the line matching your board by removing the //

//#define TTGO_V1
//#define TTGO_V2
//#define HELTEC
```

Connect the board to the computer, select your board in the Arduino IDE `Tools > Boards `

![Select board on Arduino IDE](/doc/images/select_board_arduino.png "Select board on Arduino IDE")

Then select the port where the board is connected to the computer in `Tools > Ports`

And finally click on the rounded arrow button on the top to upload the project to the board or go to `Program > Upload (Ctl+U)`

# Configure Station parameters
The first time the board boot it will generate an AP with the name: FossaGroundStation. Once connected to that network you should be prompted with a web panel to configure the basic parameters of your station. If that were not the case, you can access the web panel using a web browser and going to the url 192.168.4.1.

<p float="left" align="center">
  <img src="/doc/images/config_ap.jpg" width="400" />
  <img src="/doc/images/config_wifimanager.jpg" width="400" /> 
</p>

The parameters that must be filled are the following:
* **GROUNDSTATION NAME:** The name of your ground station. If you have registered yours in the [Fossa Ground Station Database](http://groundstationdatabase.com/database.php), the name should match. All ground stations starting with `test_` will be considered stations in test mode and all messages from those gs will be published on the [TEST telegram channel](https://t.me/FOSSASAT_TEST) in order not to flood the main channel with test packaged. Make sure you use a name stating with `test_` for example if you are going to test you gs with a satellite emulator.
 **GROUNDSTATION PASSWORD:** This is the password of your ground station, you will be asked for this password next time you connect to it through the web panel. **The user is always `admin`**
* **SSID and PASSWORD:** The configuration parameters of you home WiFi AP so that the ground station can connect to internet.
* **TIME ZONE:** Your timezone, this is to show you the time imformation in your timezone.
* **LATITUDE and LONGITUDE:** The geographical coordinates of the ground station. This serves the purpose of locating your ground station when you receive a package from the satellite.
* **MQTT_SERVER and MQTT_PORT:** These are the address and port of the MQTT server of the project you should not change them if you want the Ground Station to be able to connect the main server. 
* **MQTT_USER and MQTT_PASS:** These are the credentials of the project MQTT server, the purpose is to be able to collect the most packets from the satellite and manage all groundStations from this central server. You can ask for user and password in this telegram group: https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q
* **BOARD TYPE:** The hardware board you are using. The firmware is able to autodetect your board but, in case the selection is wrong or you know what you are doing, you can change it manually by modifying this parameter.

# OTA Update
This project implements OTA updates with both Arduino IDE and Platformio. To use this method the board and the computer have to be connected to the same network and be visible to each other.

## Platformio
In order to upload a new version through OTA in platformio, the `platformio.ide` file has to be edited uncommenting two lines to enable OTA and set the current IP Address of the station (it can be seen on the OLED display).

```
# Uncomment these 2 lines by deleting ";" and edit as needed to upload through OTA
;upload_protocol = espota
;upload_port = IP_OF_THE_BOARD
```

Once this is done, the new firmware can be uploaded using the upload button normally as if the board were connected through USB.

## Arduino IDE
To upload a new version through OTA on Arduino, you have to navigate to `Tools > port` and, if the computer is in the same network it should detect a network port for the ESP32. If that is the case, select the network port.

Once this is done, the new firmware can be uploaded normally using the upload button or navigating to `Program > upload`
Arduino 


