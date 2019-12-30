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
	mqtt_cfg.transport = MQTT_TRANSPORT_OVER_SSL;
#else
	mqtt_cfg.transport = MQTT_TRANSPORT_OVER_TCP;
#endif // SECURE_MQTT
	mqtt_cfg.event_handle = mqtt_event_handler;
	mqtt_cfg.user_context = this;
	//ESP_LOGI (MQTT_TAG, "this = %p", this);
}

bool Esp32_mqtt_clientClass::begin () {
	esp_err_t err;
#ifdef SECURE_MQTT
	err = esp_tls_set_global_ca_store (DSTroot_CA, sizeof (DSTroot_CA));
	ESP_LOGI (MQTT_TAG, "CA store set. Error = %d %s", err, esp_err_to_name (err));
#endif // SECURE_MQTT
	client = esp_mqtt_client_init (&mqtt_cfg);
	err = esp_mqtt_client_start (client);
	ESP_LOGI (MQTT_TAG, "Client connect. Error = %d %s", err, esp_err_to_name (err));

	return err == ESP_OK;
}

bool Esp32_mqtt_clientClass::setLastWill (const char* topic) {
	mqtt_cfg.lwt_topic = topic;
	mqtt_cfg.lwt_msg = "0";
	mqtt_cfg.lwt_msg_len = 1;
	//mqtt_cfg.lwt_qos = qos;
	mqtt_cfg.lwt_retain = true;

	return true;
}

bool Esp32_mqtt_clientClass::publish (const char* topic, const char* payload, size_t payload_lenght, int qos, bool retain) {
	return esp_mqtt_client_publish (client, topic, (char*)payload, payload_lenght, qos, retain);
}

esp_err_t Esp32_mqtt_clientClass::mqtt_event_handler (esp_mqtt_event_handle_t event) {
	Esp32_mqtt_clientClass* client = (Esp32_mqtt_clientClass*)(event->user_context);

	if (event->event_id == MQTT_EVENT_CONNECTED) {
		ESP_LOGI (MQTT_TAG, "MQTT msgid= %d event: %d. MQTT_EVENT_CONNECTED", event->msg_id, event->event_id);
		//esp_mqtt_client_subscribe (((Esp32_mqtt_clientClass*)event)->client, "test/hello", 0);
		if (client->mqtt_cfg.lwt_topic) {
			esp_mqtt_client_publish (
				client->client, client->mqtt_cfg.lwt_topic, "1", 1, 0, true);
		}
	} else if (event->event_id == MQTT_EVENT_DISCONNECTED) {
		ESP_LOGI (MQTT_TAG, "MQTT event: %d. MQTT_EVENT_DISCONNECTED", event->event_id);
		//esp_mqtt_client_reconnect (event->client); //not needed if autoconnect is enabled
	} else  if (event->event_id == MQTT_EVENT_SUBSCRIBED) {
		ESP_LOGI (MQTT_TAG, "MQTT msgid= %d event: %d. MQTT_EVENT_SUBSCRIBED", event->msg_id, event->event_id);
	} else  if (event->event_id == MQTT_EVENT_UNSUBSCRIBED) {
		ESP_LOGI (MQTT_TAG, "MQTT msgid= %d event: %d. MQTT_EVENT_UNSUBSCRIBED", event->msg_id, event->event_id);
	} else  if (event->event_id == MQTT_EVENT_PUBLISHED) {
		ESP_LOGI (MQTT_TAG, "MQTT event: %d. MQTT_EVENT_PUBLISHED", event->event_id);
	} else  if (event->event_id == MQTT_EVENT_DATA) {
		ESP_LOGI (MQTT_TAG, "MQTT msgid= %d event: %d. MQTT_EVENT_DATA", event->msg_id, event->event_id);
		ESP_LOGI (MQTT_TAG, "Topic length %d. Data length %d", event->topic_len, event->data_len);
		ESP_LOGI (MQTT_TAG, "Incoming data: %.*s %.*s", event->topic_len, event->topic, event->data_len, event->data);
		//ESP_LOGI (MQTT_TAG, "client->_onData %p", client->_onData);
		if (client->_onData) {
			ESP_LOGI (MQTT_TAG,"Data event!!!");
			client->_onData (event->topic, event->topic_len, event->data, event->data_len);
		}
	} else  if (event->event_id == MQTT_EVENT_BEFORE_CONNECT) {
		ESP_LOGI (MQTT_TAG, "MQTT event: %d. MQTT_EVENT_BEFORE_CONNECT", event->event_id);
	}
	if (event->event_id != MQTT_EVENT_DATA) {
		if (client->_onEvent) {
			//ESP_LOGI (MQTT_TAG, "Event!");
			//ESP_LOGI (MQTT_TAG, "event->user_context = %p", client);
			//ESP_LOGI (MQTT_TAG, "event->user_context->_onevent = %p", client->_onEvent);

			client->_onEvent (event->event_id);
		}
	}
	return ESP_OK;
}

void Esp32_mqtt_clientClass::onEvent (onMQTTevent_t event_callback) {
	_onEvent = event_callback;
	//ESP_LOGI (MQTT_TAG,"Event callback %p %p", _onEvent, event_callback);
}

void Esp32_mqtt_clientClass::onReceive (onMQTTdata_t data_callback) {
	_onData = data_callback;
	//ESP_LOGI (MQTT_TAG,"Data callback %x %x", _onData, data_callback);
}


bool Esp32_mqtt_clientClass::subscribe (const char* topic, int32_t qos) {
	return esp_mqtt_client_subscribe (client, topic, qos) >= 0;
}


//Esp32_mqtt_clientClass Esp32_mqtt_client;
