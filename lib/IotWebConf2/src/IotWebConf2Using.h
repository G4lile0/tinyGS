/**
 * IotWebConf2Using.h -- IotWebConf2 is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf2
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef IotWebConf2Using_h
#define IotWebConf2Using_h

// This "using" lines are just aliases, and should avoided.

using IotWebConf2ParameterGroup = iotwebconf2::ParameterGroup;
using IotWebConf2TextParameter = iotwebconf2::TextParameter;
using IotWebConf2PasswordParameter = iotwebconf2::PasswordParameter;
using IotWebConf2NumberParameter = iotwebconf2::NumberParameter;
using IotWebConf2CheckboxParameter = iotwebconf2::CheckboxParameter;
using IotWebConf2SelectParameter = iotwebconf2::SelectParameter;

#endif