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
* **T-BEAM + OLED** (433MHz SX1278)
* **ESP32 dev board + SX126X with crystal (Custom build, OLED optional)**
* **ESP32 dev board + SX126X with TCXO (Custom build, OLED optional)**
* **ESP32 dev board + SX127X (Custom build, OLED optional)**
* **TTGO LoRa32 V2 (Manualy swaped SX1267 to SX1278)**

## Supported modules
* sx126x
* sx127x

<p align="center">
<img src="/doc/images/Heltec.jpg" width="300">
</p>

# Quick Install
This project is ready to use with [Platformio](https://platformio.org/). It will take care of all dependencies automatically when building the project. It can also be used with [Arduino IDE](https://github.com/G4lile0/ESP32-OLED-Fossa-GroundStation/wiki/Arduino-IDE).

## Platformio (strongly recommended)
### Installing platformio
Platformio can be installed as a plugin for many IDEs. You can find a complete list here: https://docs.platformio.org/en/latest/ide.html#desktop-ide

My recommendation is to use VSCode or Atom, you can find installation guides here:

* VSCode: https://docs.platformio.org/en/latest/ide/vscode.html
* Atom: https://docs.platformio.org/en/latest/ide/atom.html

### Open the project in VSCode
Once you have cloned this project to a local directory, you can open it on Visual Studio code in File > Add folder to workspace.

![Add folder to workspace VSCode](/doc/images/add_folder_to_workspace.png "Add folder to workspace VSCode")

Then select the `ESP32-OLED-Fossa-GroundStation` folder of the repository and click open, make sure it is the root folder and that it has the platformio.ino inside.

![Select folder](/doc/images/Select_folder.png "Select folder")

After that, the project should be loaded in visual studio and ready to configure and build. **There is no need to change anything on the code if you have one of the supported boards. Platformio will compile for heltec_wifi_lora_32 but that is normal even if your board is not Heltec.**

## Build and upload the project
Connect the board to the computer and click on the upload button from the platformio toolbar, or go to Terminal -> Run Task -> Upload.

![Upload](/doc/images/upload.png "Upload")

All the dependencies will be configured and built automatically.

Note that if you are a Linux used like me and it is your first time using platformio, you will have to install the udev rules to grant permissions to platformio to upload the program to the board. You can follow the instructions here: https://docs.platformio.org/en/latest/faq.html#platformio-udev-rules

# Configure Station parameters
The first time the board boot it will generate an AP with the name: FossaGroundStation. Once connected to that network you should be prompted with a web panel to configure the basic parameters of your station. If that were not the case, you can access the web panel using a web browser and going to the url 192.168.4.1.

<p float="left" align="center">
  <img src="/doc/images/config_ap.jpg" width="400" />
  <img src="/doc/images/config_wifimanager.jpg" width="400" /> 
</p>

You can find more information about the config parameters and the available boards on [the configuration wiki page](https://github.com/G4lile0/ESP32-OLED-Fossa-GroundStation/wiki/Ground-Station-configuration).

# OTA Update
This project implements OTA updates with both Arduino IDE and Platformio. To use this method the board and the computer have to be connected to the same network and be visible to each other.

You can find more information on [the wiki page about the OTA Update](https://github.com/G4lile0/ESP32-OLED-Fossa-GroundStation/wiki/OTA-Update).

# Dependencies
This project relies on several third party dependencies that must be installed in order to be able to build the binaries. You can find the dependencies list below.

* **RadioLib (with modifications)** (**required:** v3.0.0@4m1g0) https://github.com/4m1g0/RadioLib
* **ArduinoJson** (recomended v6.13.0 **Required** >v6.0) https://github.com/bblanchon/ArduinoJson
* **ESP8266_SSD1306** (recomended v4.1.0) https://github.com/ThingPulse/esp8266-oled-ssd1306
* **IoTWebConf2** (**Required:** 2.3.0@4m1g0) https://github.com/4m1g0/IotWebConf2
* **PubSubCluent (with modifications)** (recomended 2.7) https://github.com/knolleary/pubsubclient

**Note for Arduino IDE Users**: Some of this libraries have modifications compared to the original ones, so make sure you use the version listed here or just copy the libraries from the `lib`folder to avoid problems. On PubSubClient it is mandatory to set `MQTT_MAX_PACKET_SIZE` to 1000 on the `PubSubClient.h` file. Platformio users don't have to worry about this as Platformio handle all of this automatically.
