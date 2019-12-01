# ESP32-OLED-Fossa-GroundStation for Platformio
Groundstation for the the Fossasat-1 Satellite 

Based on [@G4lile0](https://github.com/G4lile0/ESP32-OLED-Fossa-GroundStation) [ESP32-OLED-Fossa-GroundStation](https://github.com/G4lile0/ESP32-OLED-Fossa-GroundStation) and adapted to platformio by [@4m1g0](https://github.com/4m1g0).

## Supported boards
* **Heltec WiFi LoRa 32** https://heltec.org/project/wifi-kit-32/
* **TTGO LoRa32 V1** 

<p align="center">
<img src="/doc/images/Heltec.jpg" width="300">
</p>

# Quick Install
This project is ready to use with [Platformio] (https://platformio.org/). It will take care of all dependencies automatically when building the project.

## Installing platformio
Platformio can be installed as a plugin for many IDEs. You can find a complete list here: https://docs.platformio.org/en/latest/ide.html#desktop-ide

My recommendation is to use VSCode or Atom, you can find installation guides here:

* VSCode: https://docs.platformio.org/en/latest/ide/vscode.html
* Atom: https://docs.platformio.org/en/latest/ide/atom.html

## Open the project in VSCode
Once you have cloned this project to a local directory, you can open it on Visual Studio code in File > Add folder to workspace.

![Add folder to workspace VSCode](/doc/images/add_folder_to_workspace.png "Add folder to workspace VSCode")

Then select the ESP32-OLED-Fossa-GroundStation folder inside the repository and click open.

![Select folder](/doc/images/Select_folder.png "Select folder")

After that, the project should be loaded in visual studio and ready to configure and build.

## Configure the project
First we need to select the board. To do so, open the platformio.ini file and uncomment one of the lines at the beggining of the file.

Then we have to edit a bunch of parameters of the ground stations such as its name, coordinates and the information to connect to a WiFi access point.

To do so, go to the Config folder inside of the project and rename the file Config.h.dist to > Config.h (or duplicate the file and rename it).

![Configure the project](/doc/images/config.png "Configure the project")

Now you have several parameters to edit:
* **STATION_ID:** The name of your ground station. If you have registered yours in the [Fossa Ground Station Database](http://groundstationdatabase.com/database.php), the name should match.
* **LATITUDE and LONGITUDE:** The geographical coordinates of the ground station. This serves the purpose of locating your ground station when you receive a package from the satellite.
* **WIFI_SSID and WIFI_PASSWORD:** The configuration parameters of you WiFi AP so that the ground station can connect to internet.
* **MQTT_USER and MQTT_PASS:** These are the credentials of @G4lile0's MQTT server, the purpose is to be able to collect the most packets from the satellite and manage all groundStations from this central server. You can ask for user and password in this telegram group: https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q 
* **MQTT_SERVER and MQTT_PORT:** These are the address and port of the @G4lile0's MQTT server you should not change them if you want the Ground Station to be able to connect the server.

## Build and upload the project
Once the configuration is done, connect the board to the computer and click on the upload button from the platformio toolbar, or go to Terminal -> Run Task -> Upload.

![Upload](/doc/images/upload.png "Upload")

All the dependencies will be downloaded and installed automatically.

Note that if you are a Linux used like me and it is your first time using platformio, you will have to install the udev rules to grant permissions to platformio to upload the progrm to the board. You can follow the instructions here: https://docs.platformio.org/en/latest/faq.html#platformio-udev-rules

# Dependencies
This project uses the following dependencies:
* **RadioLib** https://github.com/jgromes/RadioLib
* **ArduinoJson** https://github.com/bblanchon/ArduinoJson
* **ESP8266_SSD1306** https://github.com/ThingPulse/esp8266-oled-ssd1306
