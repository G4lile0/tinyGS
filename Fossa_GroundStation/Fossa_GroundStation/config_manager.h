// config_manager.h

#ifndef _CONFIG_MANAGER_h
#define _CONFIG_MANAGER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <AsyncTCP.h>
#include <FS.h>
#include <SPIFFS.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include "esp32_mqtt_client.h"
#include <functional>

constexpr auto LOG_TAG = "WIFIMAN";

constexpr auto STATION_NAME_LENGTH = 21;
constexpr auto MQTT_SERVER_LENGTH = 31;
constexpr auto MQTT_USER_LENGTH = 31;
constexpr auto MQTT_PASS_LENGTH = 31;
constexpr auto SSID_LENGTH = 30;

typedef struct {
	uint32_t crc32;
	char station[STATION_NAME_LENGTH];
	float latitude = 0.0;    // ** Beware this information is publically available use max 3 decimals 
	float longitude = 0.0;    // ** Beware this information is publically available use max 3 decimals 

	char mqtt_server_name[MQTT_SERVER_LENGTH] = "fossa.apaluba.com";
#ifdef SECURE_MQTT
	uint32_t mqtt_port = 8883;
#else
	uint32_t mqtt_port = 1883;
#endif
	char mqtt_user[MQTT_USER_LENGTH];
	char mqtt_pass[MQTT_PASS_LENGTH]; // https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q
	char ssid[SSID_LENGTH];
	uint32_t board_type;
	//char tz[40];
	int8_t tz;
} boardconfig_t;

typedef void (*onAPStarted_t)(AsyncWiFiManager* wm);
typedef void (*onConfigSaved_t)(bool result);
typedef void (*onFormat_t)(void);

class Config_managerClass
{
 protected:
	 boardconfig_t* board_config;

	 AsyncWebServer* server;
	 DNSServer* dns;
	 AsyncWiFiManager* wifiManager;

	 onAPStarted_t notifyAPStarted;
	 onConfigSaved_t notifyConfigSaved;
	 onFormat_t notifyFormat;

	 bool loadFlashData ();
	 bool saveFlashData ();
	 bool configWiFiManager ();
	 static void doSave (void);

 public:
	 Config_managerClass (boardconfig_t* config);
	 void setAPStartedCallback (onAPStarted_t cb) {
		 notifyAPStarted = cb;
	 }
	 void setConfigSavedCallback (onConfigSaved_t cb) {
		 notifyConfigSaved = cb;
	 }
	 void setFormatFlashCallback (onFormat_t cb) {
		 notifyFormat = cb;
	 }
	 bool begin();
};

#endif
