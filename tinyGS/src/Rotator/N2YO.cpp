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

#include <WiFiClientSecure.h>
#include "../certs.h"

#include "ArduinoJson.h"
#include <HttpClient.h>
#include <cppQueue.h>

#include "N2YO.h"

#define LOG_TAG "N2YO"

#if ARDUINOJSON_USE_LONG_LONG == 0 && !PLATFORMIO
#error "Using Arduino IDE is not recommended, please follow this guide https://github.com/G4lile0/tinyGS/wiki/Arduino-IDE or edit /ArduinoJson/src/ArduinoJson/Configuration.hpp and amend to #define ARDUINOJSON_USE_LONG_LONG 1 around line 68"
#endif

#include "../Logger/Logger.h"

extern cppQueue passes_queue;
extern cppQueue positions_queue;

N2YO_Client::N2YO_Client()
{
}

void N2YO_Client::printStats(void)
{
    Log::console(PSTR("[N2YO] passes_queue allocated with %lu bytes"), passes_queue.sizeOf());
    Log::console(PSTR("[N2YO] positions_queue allocated with %lu bytes"), positions_queue.sizeOf());
}

bool N2YO_Client::query_radiopasses(uint32_t norad_id, bool clearqueue)
{
    radiopasses_query_t radiopasses_query;
    memset(&radiopasses_query, 0x00, sizeof(radiopasses_query_t));

    radiopasses_query.norad_id = norad_id,
    radiopasses_query.latitude = 45.6989,
    radiopasses_query.longitude = 9.67,
    radiopasses_query.altitude = 350,
    radiopasses_query.days = 1,
    radiopasses_query.min_elevation = 0,

    strcpy(radiopasses_query.api_key, N2YO_API_KEY);

    return query_radiopasses(radiopasses_query, clearqueue);
}

/*
   Get predicted radio passes (objects to be optically visible for observers)) for any satellite relative to a location on Earth.
   This function is useful mainly for predicting satellite passes to be used for radio communications.
   The quality of the pass depends essentially on the highest elevation value during the pass, which is one of the input parameters.

   Request: /radiopasses/{id}/{observer_lat}/{observer_lng}/{observer_alt}/{days}/{min_elevation}
*/

bool N2YO_Client::query_radiopasses(radiopasses_query_t radiopasses_query, bool clearqueue)
{
    char querybuff[512];

    if (clearqueue)
    {
        passes_queue.clean();
        Log::debug(PSTR("[N2YO] radiopasses queue cleaned..."));
    }

    Log::debug(PSTR("[N2YO] querying radiopasses for NORAD id %u..."), radiopasses_query.norad_id);

    if (radiopasses_query.norad_id == 99999)
    {
        Log::debug(PSTR("[N2YO] WARNING! balloons are not yet supported...")); // set to a fixed pos ???
        return false;
    }

    n2yoClient.setCACert(n2yo_CA);

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
    {
        HTTPClient https;

        Log::debug(PSTR("[N2YO] begin..."));

        // eg: https://api.n2yo.com/rest/v1/satellite/radiopasses/47947/45.6989/9.67/150/1/0/&apiKey=XXXXXX-XXXXXX-XXXXXX-XXXX
        sprintf(querybuff, "https://api.n2yo.com/rest/v1/satellite/radiopasses/%u/%f/%f/%d/%d/%d/&apiKey=%s", radiopasses_query.norad_id, radiopasses_query.latitude, radiopasses_query.longitude, radiopasses_query.altitude, radiopasses_query.days, radiopasses_query.min_elevation, radiopasses_query.api_key);

        Log::debug(PSTR("[N2YO] current query is '%s'"), querybuff);

        if (https.begin(n2yoClient, querybuff))
        {
            Log::debug(PSTR("[N2YO] GET..."));

            // start connection and send HTTP header
            int httpCode = https.GET();

            // httpCode will be negative on error
            if (httpCode > 0)
            {
                // HTTP header has been send and Server response header has been handled
                Log::debug(PSTR("[N2YO] GET... return code: %d"), httpCode);

                // file found at server
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                {
                    // reads multiple characters and waits until there is no more data for 1 second
                    delay(3); // WARNING!
                    decodeRadiopasses(https.getString());
                }
            }
            else
            {
                Log::debug(PSTR("[N2YO] GET... failed with error: %s"), https.errorToString(httpCode).c_str());
            }

            https.end();
        }
        else
        {
            Log::debug(PSTR("[N2YO] Unable to connect"));
        }

    } // End extra scoping block (END)

    Log::debug(PSTR("[N2YO] passes-queue has %d items..."), passes_queue.getCount());

    return true;
}

/*
   Parse a radiopasses reply: 
*/

bool N2YO_Client::decodeRadiopasses(String payload)
{
    time_t utc;
    struct tm currenttime;
    struct tm passstarttime;
    struct tm passendtime;
 // struct tm passmaxtime;

    radiopass_t radiopass;

    // WARNING! update in case of changes in radiopass_t struct definition...
    // see https://arduinojson.org/v5/assistant/
    const size_t capacity = JSON_ARRAY_SIZE(PASSESQUEUE_SIZE) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + PASSESQUEUE_SIZE * JSON_OBJECT_SIZE(10);

    Log::debug(PSTR("[N2YO] GET... estimated JSON doc size: %d"), capacity);

    DynamicJsonDocument doc(capacity);

    memset(&radiopass, 0x00, sizeof(radiopass_t));

    // N2YO reply eg: {"info":{"satid":47947,"satname":"FEES","transactionscount":0,"passescount":2},"passes":[{"startAz":17.31,"startAzCompass":"NNE","startUTC":1630448555,"maxAz":98.2,"maxAzCompass":"E","maxEl":50.18,"maxUTC":1630448910,"endAz":181.59,"endAzCompass":"S","endUTC":1630449265},{"startAz":18.64,"startAzCompass":"NNE","startUTC":1630534640,"maxAz":99.66,"maxAzCompass":"E","maxEl":43.5,"maxUTC":1630534995,"endAz":178.42,"endAzCompass":"S","endUTC":1630535345}]}
    // Log::debug(PSTR("[N2YO] decoding payload: '%s'"), payload.c_str());

    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Log::debug(PSTR("[N2YO] deserializeJson() failed with code: %s"), error.c_str());
        return false;
    }

    Log::debug(PSTR("[N2YO] deserializeJson() succeeded..."));

    if (!doc.containsKey("info") || !doc.containsKey("passes"))
    {
        Log::debug(PSTR("[N2YO] missing info/passes fields..."));
        return false;
    }

    JsonVariant info = doc["info"];

    if (info.containsKey("transactionscount"))
    {
        radiopass.transaction_count = info["transactionscount"].as<int>();
        Log::debug(PSTR("[N2YO] transactionscount: %d"), radiopass.transaction_count);
    }

    if (info.containsKey("satid"))
    {
        radiopass.norad_id = info["satid"].as<long>();
        Log::debug(PSTR("[N2YO] SAT id: %d"), radiopass.norad_id);
    }

#if 0
    JsonVariant satname = info["satname"];
    if (!satname.isNull())
    {
        strncpy(radiopass.satname, satname.as<char *>(), MAXSATNAMELEN);
        Log::debug(PSTR("[N2YO] SAT name: '%s'"), radiopass.satname);
    }
#endif

    if (info.containsKey("passescount"))
    {
        radiopass.passes_count = info["passescount"].as<int>();
        Log::debug(PSTR("[N2YO] passescount: %d"), radiopass.passes_count);
    }

    JsonArray array = doc["passes"].as<JsonArray>();

    if (array.isNull())
    {
        Log::debug(PSTR("[N2YO] passes array is null"));
    }
    else
    {
        size_t passitems;

        passitems = array.size();

        // Log::debug(PSTR("[N2YO] passes array has %d items..."), passitems);

        time(&utc);
        currenttime = *gmtime(&utc);
        Log::debug(PSTR("current UTC: %04d/%02d/%02d %02d:%02d:%02d (UTC)"), 1900 + currenttime.tm_year, currenttime.tm_mon, currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);

        // timestamp is in UNIX format and is seconds; see https://www.epochconverter.com
        // WARNING! better to exit after the first or maybe the second item... no reasons for keeping too many items in the future as tinyGS will probably
        // push a new request to listen at the right time...
        for (int i = 0; i < passitems; i++)
        {
            JsonVariant pass = array[i];
            if (!pass.containsKey("startUTC") || !pass.containsKey("maxUTC") || !pass.containsKey("endUTC") || !pass.containsKey("startAz") || !pass.containsKey("maxAz") || !pass.containsKey("maxEl") || !pass.containsKey("endAz"))
                break;

            radiopass.startUTC = pass["startUTC"].as<long>();
            radiopass.endUTC = pass["endUTC"].as<long>();
            radiopass.maxUTC = pass["maxUTC"].as<long>();
            radiopass.startAz = pass["startAz"].as<float>();
            radiopass.maxAz = pass["maxAz"].as<float>();
            radiopass.maxEl = pass["maxEl"].as<float>();
            radiopass.endAz = pass["endAz"].as<float>();

            passstarttime = *gmtime(&radiopass.startUTC);
            passendtime = *gmtime(&radiopass.endUTC);
            // passmaxtime = *gmtime(&radiopass.maxUTC);

            Log::debug(PSTR("[N2YO] pass #%03d: startUTC=%ld (%04d/%02d/%02d %02d:%02d:%02d) maxUTC=%ld endUTC=%ld (%04d/%02d/%02d %02d:%02d:%02d) startAz=%f maxAz=%f maxEl=%f endAz=%f"), i,
                       radiopass.startUTC, 1900 + passstarttime.tm_year, passstarttime.tm_mon, passstarttime.tm_mday, passstarttime.tm_hour, passstarttime.tm_min, passstarttime.tm_sec, radiopass.maxUTC,
                       radiopass.endUTC, 1900 + passendtime.tm_year, passendtime.tm_mon, passendtime.tm_mday, passendtime.tm_hour, passendtime.tm_min, passendtime.tm_sec,
                       radiopass.startAz, radiopass.maxAz, radiopass.maxEl, radiopass.endAz);

            passes_queue.push(&radiopass);
        }
    }

    return true;
}

bool N2YO_Client::query_positions(uint32_t norad_id)
{
    positions_query_t positions_query;
    memset(&positions_query, 0x00, sizeof(positions_query_t));

    positions_query.norad_id = norad_id,
    positions_query.latitude = 45.6989,
    positions_query.longitude = 9.67,
    positions_query.altitude = 350,
    positions_query.seconds = POSITIONSQUEUE_SQ_SIZE, // WARNING with POSITIONSQUEUE_SIZE=512 we are using half of the space; WARNING 300s max... do multiple requests if necessary...

        strcpy(positions_query.api_key, N2YO_API_KEY);

    return query_positions(positions_query);
}

/*
Retrieve the future positions of any satellite as footprints (latitude, longitude) to display orbits on maps.
Also return the satellite's azimuth and elevation with respect to the observer location.
Each element in the response array is one second of calculation. First element is calculated for current UTC time.

Request: /positions/{id}/{observer_lat}/{observer_lng}/{observer_alt}/{seconds}
*/

bool N2YO_Client::query_positions(positions_query_t positions_query)
{
    char querybuff[512];

    Log::debug(PSTR("[N2YO] querying positions for NORAD id %u..."), positions_query.norad_id);

    if (positions_query.norad_id == 99999)
    {
        Log::debug(PSTR("[N2YO] WARNING! balloons are not yet supported...")); // set to a fixed pos ???
        return false;
    }

    n2yoClient.setCACert(n2yo_CA);

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
    {
        HTTPClient https;

        Log::debug(PSTR("[N2YO] begin..."));

        // eg: https://api.n2yo.com/rest/v1/satellite/positions/47947/45.6989/9.67/150/5/&apiKey=XXXXXX-XXXXXX-XXXXXX-XXXX
        sprintf(querybuff, "https://api.n2yo.com/rest/v1/satellite/positions/%u/%f/%f/%d/%d/&apiKey=%s", positions_query.norad_id, positions_query.latitude, positions_query.longitude, positions_query.altitude, positions_query.seconds, positions_query.api_key);

        Log::debug(PSTR("[N2YO] current query is '%s'"), querybuff);

        if (https.begin(n2yoClient, querybuff))
        {
            Log::debug(PSTR("[N2YO] GET..."));

            // start connection and send HTTP header
            int httpCode = https.GET();

            // httpCode will be negative on error
            if (httpCode > 0)
            {
                // HTTP header has been send and Server response header has been handled
                Log::debug(PSTR("[N2YO] GET... return code: %d"), httpCode);

                // file found at server
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                {
                    // reads multiple characters and waits until there is no more data for 1 second
                    delay(15); // WARNING!
                    decodePositions(https.getString());
                }
            }
            else
            {
                Log::debug(PSTR("[N2YO] GET... failed with error: %s"), https.errorToString(httpCode).c_str());
            }

            https.end();
        }
        else
        {
            Log::debug(PSTR("[N2YO] Unable to connect"));
        }

    } // End extra scoping block (END)

    Log::debug(PSTR("[N2YO] positions-queue has %d items..."), positions_queue.getCount());

    return true;
}

/*
   Parse a positions reply: 
*/

bool N2YO_Client::decodePositions(String payload)
{
    time_t utc;
    struct tm postime;
    struct tm currenttime;
    position_t position;

    // WARNING! update in case of changes in position_t struct definition...
    // see https://arduinojson.org/v5/assistant/
    const size_t capacity = JSON_ARRAY_SIZE(1 + POSITIONSQUEUE_SQ_SIZE) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + (1 + POSITIONSQUEUE_SQ_SIZE) * JSON_OBJECT_SIZE(9);

    Log::debug(PSTR("[N2YO] GET... estimated JSON doc size: %d"), capacity);

    DynamicJsonDocument doc(capacity); // WARNING! json text for 256 item is around 50k

    memset(&position, 0x00, sizeof(position_t));

    // N2YO reply eg: {"info":{"satname":"FEES","satid":47947,"transactionscount":0},"positions":[{"satlatitude":30.42417071,"satlongitude":144.83651694,"sataltitude":562.5,"azimuth":37.5,"elevation":-44.56,"ra":15.90889596,"dec":-6.24846802,"timestamp":1632922519,"eclipsed":true},{"satlatitude":30.36211626,"satlongitude":144.82133363,"sataltitude":562.48,"azimuth":37.54,"elevation":-44.59,"ra":15.8969196,"dec":-6.28436274,"timestamp":1632922520,"eclipsed":true},
    // Log::debug(PSTR("[N2YO] decoding payload: '%s'"), payload.c_str());

    DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Log::debug(PSTR("[N2YO] deserializeJson() failed with code: %s"), error.c_str());
        return false;
    }

    Log::debug(PSTR("[N2YO] deserializeJson() succeeded..."));

    if (!doc.containsKey("info") || !doc.containsKey("positions"))
    {
        Log::debug(PSTR("[N2YO] missing info/positions fields..."));
        return false;
    }

    JsonVariant info = doc["info"];

    if (info.containsKey("transactionscount"))
    {
        position.transaction_count = info["transactionscount"].as<int>();
        Log::debug(PSTR("[N2YO] transactionscount: %d"), position.transaction_count);
    }

    if (info.containsKey("satid"))
    {
        position.norad_id = info["satid"].as<long>();
        Log::debug(PSTR("[N2YO] SAT id: %d"), position.norad_id);
    }

#if 0
    JsonVariant satname = info["satname"];
    if (!satname.isNull())
    {
        strncpy(position.satname, satname.as<char *>(), MAXSATNAMELEN);
        Log::debug(PSTR("[N2YO] SAT name: '%s'"), position.satname);
    }
#endif

    JsonArray array = doc["positions"].as<JsonArray>();

    if (array.isNull())
    {
        Log::debug(PSTR("[N2YO] position array is null"));
    }
    else
    {
        size_t positems;

        positems = array.size();

        // Log::debug(PSTR("[N2YO] position array has %d items..."), positems);

        time(&utc);
        currenttime = *gmtime(&utc);
        Log::debug(PSTR("current UTC: %04d/%02d/%02d %02d:%02d:%02d (UTC)"), 1900 + currenttime.tm_year, currenttime.tm_mon, currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);

        // timestamp is in UNIX format and is seconds; see https://www.epochconverter.com
        for (int i = 0; i < positems; i++)
        {
            JsonVariant pos = array[i];
            if (!pos.containsKey("timestamp") || !pos.containsKey("satlatitude") || !pos.containsKey("satlongitude") || !pos.containsKey("sataltitude") || !pos.containsKey("azimuth") || !pos.containsKey("elevation"))
                break;

            position.timestamp = pos["timestamp"].as<long>();
            position.sat_latitude = pos["satlatitude"].as<float>();
            position.sat_longitude = pos["satlongitude"].as<float>();
            position.sat_altitude = pos["sataltitude"].as<float>();
            position.azimuth = pos["azimuth"].as<float>();
            position.elevation = pos["elevation"].as<float>();

            postime = *gmtime(&position.timestamp);

            Log::debug(PSTR("[N2YO] SAT position #%03d: timestamp=%ld UTC:%04d/%02d/%02d %02d:%02d:%02d latitude=%f longitude=%f altitude=%f azimuth=%f elevation=%f"), i, position.timestamp, 1900 + postime.tm_year, postime.tm_mon, postime.tm_mday, postime.tm_hour, postime.tm_min, postime.tm_sec, position.sat_latitude, position.sat_longitude, position.sat_altitude, position.azimuth, position.elevation);

            positions_queue.push(&position);
        }
    }

    return true;
}
