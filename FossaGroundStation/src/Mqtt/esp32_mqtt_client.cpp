/*
  esp32_mqtt_client.cpp - MQTT connection class
  
  Copyright (C) 2020 @G4lile0, @gmag12 and @dev_4m1g0

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#define MQTT_TAG "ESP32_MQTT"

#include "esp32_mqtt_client.h"
#include <WiFi.h>
#include <functional>
using namespace std;
using namespace placeholders;

void Esp32_mqtt_clientClass::init(const char* host, int32_t port, const char* user, const char* password)
{
	mqtt_cfg.host = host;
	mqtt_cfg.port = port;
	mqtt_cfg.username = user;
	mqtt_cfg.password = password;
	mqtt_cfg.keepalive = 15;
	ESP_LOGI (MQTT_TAG, "==== MQTT Configuration ====");
	ESP_LOGI (MQTT_TAG, "Host Name: %s", mqtt_cfg.host? mqtt_cfg.host:"none");
	ESP_LOGI (MQTT_TAG, "Port: %u", mqtt_cfg.port);
	ESP_LOGI (MQTT_TAG, "Username: %s", mqtt_cfg.username? mqtt_cfg.username:"none");
	ESP_LOGI (MQTT_TAG, "Password: %s", mqtt_cfg.password? mqtt_cfg.password:"none");

#ifdef SECURE_MQTT
	//mqtt_cfg.transport = MQTT_TRANSPORT_OVER_SSL;
#else
	//mqtt_cfg.transport = MQTT_TRANSPORT_OVER_TCP;
#endif // SECURE_MQTT
	mqtt_cfg.user_context = this;
#ifdef SECURE_MQTT
	espClient.setCACert(DSTroot_CA);
#endif // SECURE_MQTT
	client = new PubSubClient(espClient);
}

void Esp32_mqtt_clientClass::reconnect() {
  // Loop until we're reconnected
  while (!client->connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    setClock();
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("test/outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("test/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void Esp32_mqtt_clientClass::data_handler(char* topic, byte* payload, unsigned int length) {
	ESP_LOGI (MQTT_TAG, "MQTT data %s %.*s", topic, length, payload);
	if (_onData) {
		ESP_LOGI (MQTT_TAG,"Data event!!!");
		_onData (topic, strlen(topic), (char*)payload, length);
	}

}

bool Esp32_mqtt_clientClass::begin () {
	client->setServer(mqtt_cfg.host,mqtt_cfg.port);
	client->setCallback(std::bind(&Esp32_mqtt_clientClass::data_handler, this, _1, _2, _3));

	return true;
}

bool Esp32_mqtt_clientClass::setLastWill (const char* topic) {
	mqtt_cfg.lwt_topic = topic;
	mqtt_cfg.lwt_msg = "0";
	mqtt_cfg.lwt_msg_len = 1;
	mqtt_cfg.lwt_retain = true;

	return true;
}

bool Esp32_mqtt_clientClass::publish (const char* topic, const char* payload, size_t payload_lenght, int qos, bool retain) {
	return client->publish (topic, (uint8_t*)payload, payload_lenght, retain);
}

void Esp32_mqtt_clientClass::onReceive (onMQTTdata_t data_callback) {
	_onData = data_callback;
	//ESP_LOGI (MQTT_TAG,"Data callback %x %x", _onData, data_callback);
}


bool Esp32_mqtt_clientClass::subscribe (const char* topic) {
	String new_topic (topic);
	subs_topics.push_back(new_topic);
	if (client->connected()){
		client->subscribe(topic);
	}
	//return esp_mqtt_client_subscribe (client, topic, qos) >= 0;
	return true;
}

bool Esp32_mqtt_clientClass::unsubscribe (const char* topic) {
	String new_topic (topic);
	for (auto it = subs_topics.begin();it!=subs_topics.end();++it){
		if (it->equals(topic)){
			subs_topics.erase(it);
		}
	}
	return true;
}

Esp32_mqtt_clientClass::~Esp32_mqtt_clientClass(){
	if (client) {
		client->disconnect();
	}
	delete client;
}
//Esp32_mqtt_clientClass Esp32_mqtt_client;
