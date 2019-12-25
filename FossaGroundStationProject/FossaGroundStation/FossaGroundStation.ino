#include <Arduino.h>
#include "ConfigManager/ConfigManager.h"

ConfigManager configManager;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");
  configManager.init();
}

void loop() {
  configManager.doLoop();
}