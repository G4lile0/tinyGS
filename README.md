<p align="center">
<img src="/doc/images/TinyGS_logo.png" width="600">
</p>

TinyGS is an open network of Ground Stations distributed around the world to receive and operate LoRa satellites, weather probes and other flying objects, using cheap and versatile modules.

# Hardware

This project is based on ESP32 boards and currently it is compatible with sx126x and sx127x LoRa módules but we plan to support more radio módules in the future.

Currently we are officially supporting the following proven LoRa boards:

- **Heltec WiFi LoRa 32 V1 (433MHz &amp; 863-928MHz versions)**
- **Heltec WiFi LoRa 32 V2 (433MHz &amp; 863-928MHz versions)**
- **TTGO LoRa32 V1**** (433MHz &amp; 868-915MHz versions)**
- **TTGO LoRa32 V2 (433MHz &amp; 868-915MHz versions)**
- **TTGO LoRa32 V2 (Manually swapped SX1267 to SX1278)**
- **T-BEAM + OLED**** (433MHz &amp; 868-915MHz versions)**
- **T-BEAM V1.0 + OLED**
- **FOSSA 1W Ground Station (433MHz &amp; 868-915MHz versions)**
- **ESP32 dev board + SX126X with crystal (Custom build, OLED optional)**
- **ESP32 dev board + SX126X with TCXO (Custom build, OLED optional)**
- **ESP32 dev board + SX127X (Custom build, OLED optional)**

However, any ESP32 board with sx126x or sx127x módule can be configured using templates. You can find more info about them [here](https://github.com/G4lile0/tinyGS/wiki/Board-Templates).

# Install

The first time download the latest [release](https://github.com/G4lile0/tinyGS/releases) and flash it with PlatformIO. If you dont know PlatformIO here you have our [PlatformIO guide](https://github.com/G4lile0/tinyGS/wiki/Platformio).

Later you can update your Ground Station via [local web OTA or auto update method](https://github.com/G4lile0/tinyGS/wiki/OTA-Update).

You can also use Arduino IDE, but is a longer and hard path, because you need to install all dependencies. [Arduino guide](https://github.com/G4lile0/tinyGS/wiki/Arduino-IDE)

# Configuration

Check our wiki [configuration page](https://github.com/G4lile0/tinyGS/wiki/Ground-Station-configuration)

# Main data website

All data received by TinyGS Ground Stations are showed at our TinyGS website

[https://tinygs.com/](https://tinygs.com/)

At this web you can see:

- [Ground Stations list](https://tinygs.com/stations)
- [Supported satellites](https://tinygs.com/satellites)
- [All data packets received by the community in real time](https://tinygs.com/packets)

At your personal area you can edit some parameters of your Ground Stations remotely.

This is the main data recovery system, but we are developing an API to access data.

# Local data access

You can access to your Ground Station data and configuration via local website at your

<p align="center">
<img src="/doc/images/TinyGS_dashboard.png" width="300">
</p>

Also you can use the serial port of your board to see the basic console.

# TinyGS network architecture

<p align="center">
<img src="/doc/images/TinyGS_architecture.png" width="300">
</p>

# OTA update and Auto Tuning

Once your Ground Station is alive and connected it can be automagically updated with the last version by the server via [OTA](https://github.com/G4lile0/tinyGS/wiki/OTA-Update).

Also the Ground stations can be remote configured automagically ([Auto Tuning](https://github.com/G4lile0/tinyGS/wiki/Radio-Tuning-Guide)) to be able to hear the next satellite pass with the correct settings.

Both systems are optional and you can opt-out at your Ground Station configuration, for example if you want to only support one specific satellite. But we recommend activating both to maintain the network health.

# Community

We are using Telegram as the main communication channel for the project. There are also two channels where you can subscribe and be updated automátically whenever a new packet is received by the network from the Satellite.

- [Main community chat](https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q)
- [Data channel](https://t.me/tinyGS_Telemetry) station status and received packets
- [Test channel](https://t.me/TinyGS_Test) RX packets by groundstations in test mode

# History

When we heard about the FossaSAT-1 LoRa satellite project launch, as passionate about LoRa and space we developed &quot;ESP32-OLED-Fossa-GroundStation&quot; as a weekend project to learn an experiment about radio and space.

While we have no relation with the Fossa team in charge of the mission, we created this project to be able to track and use the satellites as well as supporting the mission.

Later the project community grow so much, and after the FossaSat-1experience, we started to support other new LoRa satellites so we renamed the project to TinyGS.

Currently the network is open to any LoRa satellite and we also support other flying objects that have a compatible radio modulation with our hardware such as FSK and GFSK for the moment.

This are the more important moments of the project

- Nov 28, 2019 ESP32-OLED-Fossa-GroundStation project born.
- Dec 6, 2019 FossaSAT-1 deployed with an Electron rocket by Rocket Lab.
- Dec 10,2019 YL3CT&#39;s GS receive the fist LoRa packet from FossaSAT-1
- Sep 28,2020 6U Norby LoRa satellite is deployed with a Soyuz-2-1b launcher
- Oct 11, 2020 KA9ETC&#39;s GS receive the first LoRa packet from Norby
- Jan 24, 2021 3x V-R3x sat deployed with a Falcon-9
- Jan 25, 2021 KA9ETC&#39;S GS receive the first LoRa packet from V-R3x
- Feb 14, 2021 New name and web tinyGS.com with a new Beta firmware.

# Contribute

You can contribute to TinyGS by

- Providing Pull Requests (Features, Proof of Concepts, Language files or Fixes)
- Testing new released features and report issues
- Contributing missing documentation for features and devices templates

# Documentation

Check our [wiki](/wiki)!

# Project dependencies

This project relies on several third party libraries:

- RadioLib
- ArduinoJson
- ESP8266\_SSD1306
- IoTWebConf2
- PubSubClient
- ESPNTPClient
- FailSafeMode

# TinyGS team

The main TinyGS developer team is:

- [4m1g0](https://github.com/4m1g0)
- [G4lile0](https://github.com/G4lile0)
- [gmag11](https://github.com/gmag11)


# License

This program is licensed under GPL-3.0
