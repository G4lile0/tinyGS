// 
// 
// 

#include "config_manager.h"

const char CONFIG_FILE[] = "/config.txt";

bool shouldSave = false;

Config_managerClass::Config_managerClass (boardconfig_t *config) {
	board_config = config;
}

bool Config_managerClass::begin()
{
	// WiFi.begin ("0", "0"); // Only for test

	if (!SPIFFS.begin ()) {
		ESP_LOGE (LOG_TAG,"Error mounting flash");
		// todo callback
		SPIFFS.format ();
		//return false;
	}
	if (!loadFlashData ()) { // Load from flash
		ESP_LOGW (LOG_TAG, "Invalid configuration");
		WiFi.begin ("0", "0"); // Reset Wifi credentials
	} else {
		ESP_LOGI (LOG_TAG, "Configuration loaded from flash");
	}

	if (configWiFiManager ()) {
		if (shouldSave) {
			ESP_LOGD (LOG_TAG,"Got configuration. Storing");
			if (saveFlashData ()) {
				ESP_LOGD (LOG_TAG, "Configuration stored on flash");
			} else {
				ESP_LOGE (LOG_TAG, "Error saving data on flash");
			}
			ESP.restart ();
		} else {
			ESP_LOGI (LOG_TAG, "Configuration has not to be saved");
		}
		return true;
	} else {
		ESP_LOGE (LOG_TAG, "Configuration error. Restarting");
		ESP.restart ();
	}
}

bool Config_managerClass::loadFlashData () {
	if (SPIFFS.exists (CONFIG_FILE)) {
		ESP_LOGD (LOG_TAG, "Opening %s file", CONFIG_FILE);
		File configFile = SPIFFS.open (CONFIG_FILE, "r");
		if (configFile) {
			ESP_LOGD (LOG_TAG, "%s opened", CONFIG_FILE);
			size_t size = configFile.size ();
			ESP_LOGD (LOG_TAG, "Config size: %d. File size %d", sizeof (boardconfig_t), configFile.size ());
			if (size != sizeof (boardconfig_t)) {
				ESP_LOGW (LOG_TAG, "Config file is corrupted. Deleting and formatting");
				SPIFFS.remove (CONFIG_FILE);
				//SPIFFS.format ();
				WiFi.begin ("0", "0"); // Delete WiFi credentials
				return false;
			}
			configFile.read ((uint8_t*)(board_config), sizeof (boardconfig_t));
			ESP_LOGD (LOG_TAG, "Config file stored station name: %s", board_config->station);
			configFile.close ();
			ESP_LOGV (LOG_TAG, "Gateway configuration successfuly read");
			ESP_LOG_BUFFER_HEX_LEVEL (LOG_TAG, board_config, sizeof (boardconfig_t), ESP_LOG_VERBOSE);

			ESP_LOGI (LOG_TAG, "==== Configuration ====");
			ESP_LOGI (LOG_TAG, "Station Name: %s", board_config->station);
			ESP_LOGI (LOG_TAG, "Latitude: %f", board_config->latitude);
			ESP_LOGI (LOG_TAG, "Longitude: %f", board_config->longitude);
			ESP_LOGI (LOG_TAG, "MQTT Server Name: %s", board_config->mqtt_server_name);
			ESP_LOGI (LOG_TAG, "MQTT Server Port: %u", board_config->mqtt_port);
			ESP_LOGI (LOG_TAG, "MQTT User: %s", board_config->mqtt_user);
			ESP_LOGI (LOG_TAG, "MQTT Password: %s", board_config->mqtt_pass);
			ESP_LOGI (LOG_TAG, "SSID: %s", board_config->ssid);

			return true;
		}
	} else {
		ESP_LOGW (LOG_TAG, "%s do not exist", CONFIG_FILE);
		//SPIFFS.format ();
		WiFi.begin ("0", "0"); // Delete WiFi credentials
		ESP_LOGW (LOG_TAG, "Dummy STA config loaded");
		WiFi.begin ("0", "0"); // Delete WiFi credentials
		return false;
	}
}

bool Config_managerClass::saveFlashData () {
	File configFile = SPIFFS.open (CONFIG_FILE, "w");
	if (!configFile) {
		ESP_LOGW (LOG_TAG, "failed to open config file %s for writing", CONFIG_FILE);
		return false;
	}
	// TODO: Add CRC
	// TODO: Check successful save
	size_t filelen = configFile.write ((uint8_t*)board_config, sizeof (boardconfig_t));
	configFile.close ();
	ESP_LOGV (LOG_TAG, "Configuration saved to flash. %u bytes", filelen);
	ESP_LOG_BUFFER_HEX_LEVEL (LOG_TAG, board_config, sizeof (boardconfig_t), ESP_LOG_VERBOSE);
	if (notifyConfigSaved) {
		notifyConfigSaved (true);
	}
	return true;
}

void Config_managerClass::doSave (void) {
	ESP_LOGI (LOG_TAG, "Configuration saving activated");
	shouldSave = true;
}


bool Config_managerClass::configWiFiManager () {
	server = new AsyncWebServer (80);
	dns = new DNSServer ();
	wifiManager = new AsyncWiFiManager (server, dns);

	char station[STATION_NAME_LENGTH] = "";
	//char networkName[NETWORK_NAME_LENGTH] = "";
	char latitude[10];
	snprintf (latitude, sizeof (latitude), "%.2f", board_config->latitude);
	char longitude[10] = "0.0";
	snprintf (longitude, sizeof (longitude), "%.2f", board_config->longitude);
	char mqtt_port[6];
	snprintf (mqtt_port, sizeof (mqtt_port), "%u", board_config->mqtt_port);

	AsyncWiFiManagerParameter stationNameParam ("station_name", "Station Name", board_config->station, STATION_NAME_LENGTH - 1, "required type=\"text\" maxlength=20");
	AsyncWiFiManagerParameter latitudeParam ("lat", "Latitude", latitude, 9, "required type=\"number\" min=\"-180\" max=\"180\" step=\"0.001\"");
	AsyncWiFiManagerParameter longitudeParam ("lon", "Longitude", longitude, 9, "required type=\"number\" min=\"-180\" max=\"180\" step=\"0.001\"");
	AsyncWiFiManagerParameter mqttServerNameParam ("server_name", "MQTT Server Name", board_config->mqtt_server_name, MQTT_SERVER_LENGTH - 1, "required type=\"text\" maxlength=30");
	AsyncWiFiManagerParameter mqttServerPortParam ("server_port", "MQTT Server Port", mqtt_port, 5, "required type=\"number\" min=\"0\" max=\"65536\" step=\"1\"");
	AsyncWiFiManagerParameter mqttUserParam ("user", "MQTT User Name", board_config->mqtt_user, MQTT_USER_LENGTH - 1, "required type=\"text\" maxlength=30");
	AsyncWiFiManagerParameter mqttPassParam ("pass", "MQTT Password", "", MQTT_PASS_LENGTH - 1, "type=\"password\" maxlength=30");

	wifiManager->addParameter (&stationNameParam);
	wifiManager->addParameter (&latitudeParam);
	wifiManager->addParameter (&longitudeParam);
	wifiManager->addParameter (new AsyncWiFiManagerParameter ("<br>"));
	wifiManager->addParameter (&mqttServerNameParam);
	wifiManager->addParameter (&mqttServerPortParam);
	wifiManager->addParameter (&mqttUserParam);
	wifiManager->addParameter (&mqttPassParam);

	//if (notifyWiFiManagerStarted) {
	//	notifyWiFiManagerStarted ();
	//}

	wifiManager->setDebugOutput (true);
	//wifiManager->setBreakAfterConfig (true);
	wifiManager->setTryConnectDuringConfigPortal (false);
	wifiManager->setSaveConfigCallback (doSave);
	wifiManager->setConfigPortalTimeout (150);

	if (notifyAPStarted) {
		wifiManager->setAPCallback (notifyAPStarted);
	}

	boolean result = wifiManager->autoConnect ("FossaGroundStation");
	ESP_LOGI (LOG_TAG, "==== Config Portal result ====");
	ESP_LOGI (LOG_TAG, "Station Name: %s", stationNameParam.getValue ());
	ESP_LOGI (LOG_TAG, "Latitude: %.*s", latitudeParam.getValueLength(), latitudeParam.getValue ());
	ESP_LOGI (LOG_TAG, "Longitude: %.*s", longitudeParam.getValueLength (), longitudeParam.getValue ());
	ESP_LOGI (LOG_TAG, "MQTT Server Name: %s", mqttServerNameParam.getValue ());
	ESP_LOGI (LOG_TAG, "MQTT Server Port: %s", mqttServerPortParam.getValue ());
	ESP_LOGI (LOG_TAG, "MQTT User: %s", mqttUserParam.getValue ());
	ESP_LOGI (LOG_TAG, "MQTT Password: %s", mqttPassParam.getValue ());
	ESP_LOGI (LOG_TAG, "Status: %s", result ? "true" : "false");
	ESP_LOGI (LOG_TAG, "Save config: %s", shouldSave ? "yes" : "no");
	if (result) {
		if (shouldSave) {
			memcpy (board_config->station, stationNameParam.getValue (), stationNameParam.getValueLength ());
			board_config->latitude = atof (latitudeParam.getValue ());
			board_config->longitude = atof (longitudeParam.getValue ());

			memcpy(board_config->mqtt_server_name, mqttServerNameParam.getValue (), mqttServerNameParam.getValueLength ());
			board_config->mqtt_port = atoi (mqttServerPortParam.getValue ());
			memcpy(board_config->mqtt_user, mqttUserParam.getValue (), mqttUserParam.getValueLength ());
			if (strcmp(mqttPassParam.getValue (),"")) {
				memcpy (board_config->mqtt_pass, mqttPassParam.getValue (), mqttPassParam.getValueLength ());
			} else {
				ESP_LOGI (LOG_TAG, "Password not changed");
			}

			String wifiSSID = WiFi.SSID ();
			memcpy(board_config->ssid, wifiSSID.c_str (), wifiSSID.length() > SSID_LENGTH ? SSID_LENGTH : wifiSSID.length ());
		} else {
			ESP_LOGD (LOG_TAG, "Configuration does not need to be saved");
		}
	} else {
		ESP_LOGE (LOG_TAG, "WiFi connection unsuccessful. Restarting");
		//ESP.restart ();
	}

	//if (notifyWiFiManagerExit) {
	//	notifyWiFiManagerExit (result);
	//}

	free (server);
	free (dns);
	free (wifiManager);

	return result;
}
