// arduino_ota.h

#ifndef _ARDUINO_OTA_h
#define _ARDUINO_OTA_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <ESPmDNS.h>
#include <Update.h>
#include <ArduinoOTA.h>

void arduino_ota_setup ();


#endif
