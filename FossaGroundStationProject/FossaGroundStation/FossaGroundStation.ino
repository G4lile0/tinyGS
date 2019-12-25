#include <Arduino.h>
#include "ConfigManager/ConfigManager.h"

ConfigManager configManager;

void wifiConnected() {
  Serial.print("MQTT Port: ");
  Serial.println(configManager.getMqttPort());
  Serial.print("MQTT Server: ");
  Serial.println(configManager.getMqttServer());
  Serial.print("MQTT Pass: ");
  Serial.println(configManager.getMqttPass());
  Serial.print("Latitude: ");
  Serial.println(configManager.getLatitude());
  Serial.print("Longitude: ");
  Serial.println(configManager.getLongitude());
  Serial.print("tz: ");
  Serial.println(configManager.getTZ());
  Serial.print("board: ");
  Serial.println(configManager.getBoard());
  
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");
  configManager.setWifiConnectionCallback(wifiConnected);
  configManager.init();
}

void loop() {
  configManager.doLoop();
}