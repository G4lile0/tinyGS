/*
  N2YO.h - Class to handle communications with n2yo rest API
  
  Copyright (C) 2021 @iw2lsi

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

#ifndef N2YO_H
#define N2YO_H

#include "../ConfigManager/ConfigManager.h"
#include "../Status.h"

#if MQTT_MAX_PACKET_SIZE != 1000 && !PLATFORMIO
#error "Using Arduino IDE is not recommended, please follow this guide https://github.com/G4lile0/tinyGS/wiki/Arduino-IDE or edit /PubSubClient/src/PubSubClient.h  and set #define MQTT_MAX_PACKET_SIZE 1000"
#endif

extern Status status;

#define PASSESQUEUE_SIZE        8
#define POSITIONSQUEUE_SIZE     512 // total size
#define POSITIONSQUEUE_SQ_SIZE  64  // single query size

#define MAXSATNAMELEN       32
#define MAXAPIKEYLEN        32
#define N2YO_API_KEY        "3NXG3F-XCPJAT-2XQ6KZ-4RPQ"  // WARNING make it configurable

typedef struct radiopasses_query_t
{

  uint32_t norad_id;
  float latitude;
  float longitude;
  int altitude;
  int days;
  int min_elevation;
  char api_key[MAXAPIKEYLEN];

} radiopasses_query_t;

// WARNING! update capacity evaluation in decodePositions() in case of changes in radiopass_t struct!

typedef struct radiopass_t
{

  uint32_t norad_id;
//char satname[MAXSATNAMELEN + 1];
  uint16_t transaction_count;
  uint16_t passes_count;
  float startAz;
  time_t startUTC;
  float maxAz;
  float maxEl;
  time_t maxUTC;
  float endAz;
  time_t endUTC;

} radiopass_t;

typedef struct positions_query_t
{

  uint32_t norad_id;
  float latitude;
  float longitude;
  int altitude;
  int seconds; // WARNING 300s max... do multiple requests if necessary...
  char api_key[MAXAPIKEYLEN + 1];

} positions_query_t;

// WARNING! update capacity evaluation in decodeRadiopasses() in case of changes in position_t struct!

typedef struct position_t
{

  uint32_t norad_id;
//char satname[MAXSATNAMELEN + 1];
  uint16_t transaction_count;
  float sat_latitude;
  float sat_longitude;
  float sat_altitude;
  float azimuth;
  float elevation;
  time_t timestamp;

} position_t;

class N2YO_Client
{
public:

  static N2YO_Client &getInstance()
  {
    static N2YO_Client instance;
    return instance;
  }

  void printStats(void);

  bool query_radiopasses(uint32_t norad_id, bool clearqueue);
  bool query_radiopasses(radiopasses_query_t radiopasses_query, bool clearqueue);

  bool query_positions(uint32_t norad_id);
  bool query_positions(positions_query_t positions_query);

protected:

  WiFiClientSecure n2yoClient;

private:

  N2YO_Client();

  bool decodeRadiopasses(String payload);
  bool decodePositions(String payload);
};

#endif
