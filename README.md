#  tinyGS Open Ground Station Network
TinyGS is an open network of Ground Stations distributed around the world to receive and operate LoRa satellites and weather probes, using cheap and versatile modules.

This project is based on ESP32 boards and currently it is compatible with sx126x and sx127x but we plan to support more in the future. You can build your own board using one of these modules but most of us use a development board like the ones listed below.

# Supported boards
* **Heltec WiFi LoRa 32 V1** 
* **Heltec WiFi LoRa 32 V2** 
* **TTGO LoRa32 V1** 
* **TTGO LoRa32 V2** 
* **T-BEAM + OLED** 
* **ESP32 dev board + SX126X with crystal (Custom build, OLED optional)**
* **ESP32 dev board + SX126X with TCXO (Custom build, OLED optional)**
* **ESP32 dev board + SX127X (Custom build, OLED optional)**
* **TTGO LoRa32 V2 (Manualy swaped SX1267 to SX1278)**

**Important** verify that the board that you chosse support the band of the satellites that you want to receive. 

## Supported modules
* sx126x
* sx127x

<p align="center">
<img src="/doc/images/Heltec.jpg" width="300">
</p>


Initially it was developed as a "weekend" project for the FossaSAT-1 LoRa satellite. We are passionate about space and created this project to be able to track and use the satellites to learn an experiment about …- Currently the network is open to any LoRa satellite and we also support other flying objects that have a compatible radio modulation with our hardware such as FSK, GFSK, LR-FHSS …..

It allows the operator of the Satellites/Probes to receive and operate their devices across the globe rather than just only when they fly over their location .

We are using Telegram as the mean of communication for the project, there are also two channels where you can suscribe and be updated automátically whenever a new packet is received by the network from the Satellite.

* **Main community chat:** https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q
* **Data channel (station status and received packets):** https://t.me/FOSSASAT_DATA
* **Test channel (simulator packets received by test groundstations):** https://t.me/FOSSASAT_TEST

## **Milestones:**

* Nov 28, 2019  ESP32-OLED-Fossa-GroundStation    project born.
* Dec  6, 2019   FossaSAT-1 deployed with an Electron rocket by Rocket Lab.
* Dec 10,2019     YL3CT’s GS receive the fist LoRa packet from FossaSAT-1
* Sep 28,2020    6U Norby LoRa satellite is deployed with a Soyuz-2-1b launcher
* Oct 11, 2020    KA9ETC’s GS receive the first LoRa packet from Norby
* Jan 24, 2021    3x V-R3x sat deployed with a Falcon-9
* Jan 25, 2021    KA9ETC’S GS receive the first LoRa packet from V-R3x
* Feb 14, 2021    New name and web tinyGS.com with a new Beta firmware.

## **Upcoming LoRa Satellites**

* Feb 28,2021    SD SAT will launch by ISRO PSLV rocket 
* Mar 15, 2021    FossaSat 1B, FossaSat 2 & NPS-CENETIX-Orbital 1 with Firefly


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
