// config_manager.h

#ifndef _CONFIG_MANAGER_h
#define _CONFIG_MANAGER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
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

typedef struct {
	char station[20];
	float latitude;    // ** Beware this information is publically available use max 3 decimals 
	float longitude;    // ** Beware this information is publically available use max 3 decimals 

	char mqtt_server_name[30];
#ifdef SECURE_MQTT
	uint32_t mqtt_port = 8883;
#else
	uint32_t mqtt_port = 1883;
#endif
	char mqtt_user[30];
	char mqtt_pass[30]; // https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q 
} boardconfig_t;


class Config_managerClass
{
 protected:


 public:
	void init();
};

extern Config_managerClass Config_manager;

#endif

