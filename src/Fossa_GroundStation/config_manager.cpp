// 
// 
// 

#include "config_manager.h"
#define WIFI_CONNECT_TIMEOUT 10000

constexpr auto WIFIMAN_TAG = "WIFIMAN";

const char CONFIG_FILE[] = "/config.json";

bool shouldSave = false;

Config_managerClass::Config_managerClass (boardconfig_t *config) {
	board_config = config;
}

bool Config_managerClass::begin(bool invalidate_config)
{
	// WiFi.begin ("0", "0"); // Only for test
	//SPIFFS.format (); // Only for test

	if (!SPIFFS.begin ()) {
		ESP_LOGE (WIFIMAN_TAG,"Error mounting flash");
		if (notifyFormat) {
			notifyFormat ();
		}
		SPIFFS.format ();
	}
	if (!loadFlashData (!invalidate_config)) { // Load from flash
		ESP_LOGW (WIFIMAN_TAG, "Invalid configuration");
		strlcpy(board_config->ssid, "0\0", 2);
		WiFi.begin ("0"); // Reset Wifi credentials
	} else {
		ESP_LOGI (WIFIMAN_TAG, "Configuration loaded from flash");
		if (invalidate_config)
			WiFi.begin ("0"); // Reset Wifi credentials
		else
			WiFi.begin (board_config->ssid, board_config->pass);
		time_t start_connect = millis ();
		while (millis () - start_connect > WIFI_CONNECT_TIMEOUT) {
			Serial.print ('.');
			delay (250);
		}
	}

	if (configWiFiManager ()) {
		if (shouldSave) {
			ESP_LOGD (WIFIMAN_TAG,"Got configuration. Storing");
			if (saveFlashData ()) {
				ESP_LOGD (WIFIMAN_TAG, "Configuration stored on flash");
			} else {
				ESP_LOGE (WIFIMAN_TAG, "Error saving data on flash");
			}
			ESP.restart ();
		} else {
			ESP_LOGI (WIFIMAN_TAG, "Configuration has not to be saved");
		}
		return true;
	} else {
		ESP_LOGE (WIFIMAN_TAG, "Configuration error. Restarting");
		ESP.restart ();
	}
}

bool Config_managerClass::loadFlashData (bool load_wifi_data) {
	if (SPIFFS.exists (CONFIG_FILE)) {
		bool json_correct = false;

		ESP_LOGD (WIFIMAN_TAG, "Opening %s file", CONFIG_FILE);
		File configFile = SPIFFS.open (CONFIG_FILE, "r");
		if (configFile) {
			ESP_LOGD (WIFIMAN_TAG, "%s opened", CONFIG_FILE);
			size_t size = configFile.size ();
			ESP_LOGD (WIFIMAN_TAG, "Config file size %d", configFile.size ());
			// Allocate a buffer to store contents of the file.
			std::unique_ptr<char[]> buf (new char[size]);
			DynamicJsonDocument doc (512);
			DeserializationError error = deserializeJson (doc, configFile);
			if (error) {
				ESP_LOGE ("Failed to parse file");
			} else {
				ESP_LOGE ("JSON file parsed");
				json_correct = true;
			}

			
			if (doc.containsKey("station") && doc.containsKey ("latitude") && doc.containsKey ("longitude")
				&& doc.containsKey ("mqtt_server_name") && doc.containsKey ("mqtt_port")
				&& doc.containsKey ("mqtt_user") && doc.containsKey ("mqtt_port")) {
				json_correct = true;
			}

			strlcpy(board_config->station, doc["station"] | "", sizeof (board_config->station));
			board_config->latitude = doc["latitude"].as<float>();
			board_config->longitude = doc["longitude"].as<float>();
			strlcpy (board_config->mqtt_server_name, doc["mqtt_server_name"] | "", sizeof (board_config->mqtt_server_name));
			board_config->mqtt_port = doc["mqtt_port"] | 8883;
			strlcpy (board_config->mqtt_user, doc["mqtt_user"] | "", sizeof (board_config->mqtt_user));
			strlcpy (board_config->mqtt_pass, doc["mqtt_pass"] | "", sizeof (board_config->mqtt_pass));
			strlcpy (board_config->tz, doc["tz"] | "", sizeof (board_config->tz));
			if (load_wifi_data) {
				ESP_LOGD ("Loading WiFi data");
				strlcpy (board_config->ssid, doc["wifi_ssid"] | "", sizeof (board_config->ssid));
				strlcpy (board_config->pass, doc["wifi_pass"] | "", sizeof (board_config->pass));
			} else {
				ESP_LOGD ("Not loading WiFi data");
			}

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
			String output;
			serializeJsonPretty (doc, output);

			ESP_LOGD (WIFIMAN_TAG, "JSON file %s", output.c_str ());
#endif

			ESP_LOGI (WIFIMAN_TAG, "Config file stored station name: %s", board_config->station);
			configFile.close ();

			ESP_LOGI (WIFIMAN_TAG, "Gateway configuration successfuly read");
			ESP_LOG_BUFFER_HEX_LEVEL (WIFIMAN_TAG, board_config, sizeof (boardconfig_t), ESP_LOG_VERBOSE);

			ESP_LOGD (WIFIMAN_TAG, "==== Configuration ====");
			ESP_LOGD (WIFIMAN_TAG, "Station Name: %s", board_config->station);
			ESP_LOGD (WIFIMAN_TAG, "Latitude: %f", board_config->latitude);
			ESP_LOGD (WIFIMAN_TAG, "Longitude: %f", board_config->longitude);
			ESP_LOGD (WIFIMAN_TAG, "MQTT Server Name: %s", board_config->mqtt_server_name);
			ESP_LOGD (WIFIMAN_TAG, "MQTT Server Port: %u", board_config->mqtt_port);
			ESP_LOGD (WIFIMAN_TAG, "MQTT User: %s", board_config->mqtt_user);
			ESP_LOGD (WIFIMAN_TAG, "MQTT Password: %s", board_config->mqtt_pass);
			ESP_LOGD (WIFIMAN_TAG, "Time Zone: %s", board_config->tz);
			ESP_LOGD (WIFIMAN_TAG, "SSID: %s", board_config->ssid);
			ESP_LOGD (WIFIMAN_TAG, "Pass: %s", board_config->pass);

			return json_correct;
		}
	} else {
		ESP_LOGW (WIFIMAN_TAG, "%s do not exist", CONFIG_FILE);
		//SPIFFS.format ();
		WiFi.begin ("0", "0"); // Delete WiFi credentials
		ESP_LOGW (WIFIMAN_TAG, "Dummy STA config loaded");
		return false;
	}
}

bool Config_managerClass::saveFlashData () {
	SPIFFS.remove (CONFIG_FILE); // Delete existing file, otherwise the configuration is appended to the file
	File configFile = SPIFFS.open (CONFIG_FILE, "w");
	if (!configFile) {
		ESP_LOGW (WIFIMAN_TAG, "failed to open config file %s for writing", CONFIG_FILE);
		return false;
	}

	DynamicJsonDocument doc (512);

	doc["station"] = board_config->station;
	doc["latitude"] = board_config->latitude;
	doc["longitude"] = board_config->longitude;
	doc["mqtt_server_name"] = board_config->mqtt_server_name;
	doc["mqtt_port"] = board_config->mqtt_port;
	doc["mqtt_port"] = board_config->mqtt_port;
	doc["mqtt_user"] = board_config->mqtt_user;
	doc["mqtt_pass"] = board_config->mqtt_pass;
	doc["tz"] = board_config->tz;
	doc["wifi_ssid"] = board_config->ssid;
	doc["wifi_pass"] = board_config->pass;

	if (serializeJson (doc, configFile) == 0) {
		ESP_LOGE (WIFIMAN_TAG, "Failed to write to file");
	}

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
	String output;
	serializeJsonPretty (doc, output);

	ESP_LOGD (WIFIMAN_TAG, "%s", output.c_str ());
#endif

	configFile.flush ();
	size_t size = configFile.size ();
	configFile.close ();
	ESP_LOGV (WIFIMAN_TAG, "Configuration saved to flash. %u bytes", size);

	if (notifyConfigSaved) {
		notifyConfigSaved (true);
	}

	return true;
}

void Config_managerClass::doSave (void) {
	ESP_LOGI (WIFIMAN_TAG, "Configuration saving activated");
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
	AsyncWiFiManagerParameter tzParam ("tz", "TimeZone", board_config->tz , TZ_LENGTH - 1,
		"type=\"text\" maxlength=40 pattern=\"^[A-Z]{1,4}[-|+]?\\d{1,2}[A-Z]{0,5}(,[JM]\\d{1,3}(\\.\\d\\.\\d)?(\\/\\d{1,2})?(,[JM]\\d{1,3}(\\.\\d\\.\\d)?(\\/\\d{1,2})?)?)?$\"");
	AsyncWiFiManagerParameter tzHelp ("<p>Time Zone parameter can be configured as these examples: <br>\
										 <code>CET-1</code><br>\
										 <code>CET-1CEST</code><br>\
										 <code>CET-1CEST,M3.5.0,M10.5.0/3</code><br>\
										 Check <a href=\"https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv\">this list</a><link>\
										 for reference </p>");
	
	wifiManager->addParameter (&stationNameParam);
	wifiManager->addParameter (&latitudeParam);
	wifiManager->addParameter (&longitudeParam);
	wifiManager->addParameter (new AsyncWiFiManagerParameter ("<br>"));
	wifiManager->addParameter (&mqttServerNameParam);
	wifiManager->addParameter (&mqttServerPortParam);
	wifiManager->addParameter (&mqttUserParam);
	wifiManager->addParameter (&mqttPassParam);
	wifiManager->addParameter (&tzHelp);
	wifiManager->addParameter (&tzParam);

	//if (notifyWiFiManagerStarted) {
	//	notifyWiFiManagerStarted ();
	//}

	wifiManager->setDebugOutput (true);
	//wifiManager->setBreakAfterConfig (true);
	//wifiManager->setConnectTimeout (60);
	wifiManager->setTryConnectDuringConfigPortal (false);
	wifiManager->setSaveConfigCallback (doSave);
	wifiManager->setConfigPortalTimeout (300);

	if (notifyAPStarted) {
		wifiManager->setAPCallback (notifyAPStarted);
	}

	boolean result = wifiManager->autoConnect ("FossaGroundStation"/*, NULL, 10, 1000*/);
	ESP_LOGI (WIFIMAN_TAG, "==== Config Portal result ====");
	ESP_LOGI (WIFIMAN_TAG, "Station Name: %s", stationNameParam.getValue ());
	ESP_LOGI (WIFIMAN_TAG, "Latitude: %.*s", latitudeParam.getValueLength(), latitudeParam.getValue ());
	ESP_LOGI (WIFIMAN_TAG, "Longitude: %.*s", longitudeParam.getValueLength (), longitudeParam.getValue ());
	ESP_LOGI (WIFIMAN_TAG, "MQTT Server Name: %s", mqttServerNameParam.getValue ());
	ESP_LOGI (WIFIMAN_TAG, "MQTT Server Port: %s", mqttServerPortParam.getValue ());
	ESP_LOGI (WIFIMAN_TAG, "MQTT User: %s", mqttUserParam.getValue ());
	ESP_LOGI (WIFIMAN_TAG, "MQTT Password: %s", mqttPassParam.getValue ());
	ESP_LOGI (WIFIMAN_TAG, "WIFI SSID: %s", WiFi.SSID().c_str());
	ESP_LOGI (WIFIMAN_TAG, "WIFI Password: %s", WiFi.psk().c_str());
	ESP_LOGI (WIFIMAN_TAG, "Time Zone: %s", tzParam.getValue ());
		ESP_LOGI (WIFIMAN_TAG, "Status: %s", result ? "true" : "false");
	ESP_LOGI (WIFIMAN_TAG, "Save config: %s", shouldSave ? "yes" : "no");
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
				ESP_LOGI (WIFIMAN_TAG, "Password not changed");
			}
			memcpy (board_config->tz, tzParam.getValue (), tzParam.getValueLength ());

			String wifiSSID = WiFi.SSID ();
			memcpy(board_config->ssid, wifiSSID.c_str (), wifiSSID.length() > SSID_LENGTH ? SSID_LENGTH : wifiSSID.length ());
			String wifiPass = WiFi.psk ();
			memcpy (board_config->pass, wifiPass.c_str (), wifiPass.length () > PASS_LENGTH ? PASS_LENGTH : wifiPass.length ());
		} else {
			ESP_LOGD (WIFIMAN_TAG, "Configuration does not need to be saved");
		}
	} else {
		ESP_LOGE (WIFIMAN_TAG, "WiFi connection unsuccessful. Restarting");
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

void Config_managerClass::eraseConfig(){
	SPIFFS.remove (CONFIG_FILE);
	WiFi.begin ("0", "0"); // Delete WiFi credentials
}
