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

#include "N2YO.h"
#include <cppQueue.h>

#include "ArduinoJson.h"
#include <HttpClient.h>

#define LOG_TAG "N2YO"

#if ARDUINOJSON_USE_LONG_LONG == 0 && !PLATFORMIO
#error "Using Arduino IDE is not recommended, please follow this guide https://github.com/G4lile0/tinyGS/wiki/Arduino-IDE or edit /ArduinoJson/src/ArduinoJson/Configuration.hpp and amend to #define ARDUINOJSON_USE_LONG_LONG 1 around line 68"
#endif

#include "../Logger/Logger.h"

cppQueue passes_q(sizeof(radiopass_t), 10, FIFO, true);	// Instantiate queue

N2YO_Client::N2YO_Client()
{
}

bool N2YO_Client::query_n2yo(uint32_t NORAD)
{
    radiopass_t radiopass;

    memset(&radiopass, 0x00, sizeof(radiopass_t));

    DynamicJsonDocument doc(4096);
    char buffer[512];
    double lat = 45.6989; // WARNING take it from GS configuration
    double lon = 9.67;    // WARNING take it from GS configuration
    int alt = 350.0;      // WARNING take it from GS configuration
    int seconds = 5;      // WARNING 300s max... use multiple requests ?

    int days = 1;     //
    int min_elev = 0; // minimum elevation

    const char api_key[] = "3NXG3F-XCPJAT-2XQ6KZ-4RPQ"; // WARNING make it configurable

    Log::debug(PSTR("[N2YO] querying NORAD %u..."), NORAD);

    if (NORAD == 99999)
    {
        Log::debug(PSTR("[N2YO] WARNING! balloons are not yet supported...")); // set to a fixed pos ???
        return false;
    }

    WiFiClientSecure *client = new WiFiClientSecure;

    if (!client)
    {
        Log::debug(PSTR("[N2YO] Unable to create client"));
        return false;
    }

    client->setCACert(n2yo_CA);

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
    {
        HTTPClient https;

        Log::debug(PSTR("[N2YO] begin..."));

        // eg: https://api.n2yo.com/rest/v1/satellite/radiopasses/47947/45.6989/9.67/150/1/0/&apiKey=XXXXXX-XXXXXX-XXXXXX-XXXX
        sprintf(buffer, "https://api.n2yo.com/rest/v1/satellite/radiopasses/%u/%f/%f/%d/%d/%d/&apiKey=%s", NORAD, lat, lon, alt, days, min_elev, api_key);

        Log::debug(PSTR("[N2YO] current query is '%s'"), buffer);

        if (https.begin(*client, buffer))
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
                    String payload = https.getString();

                    Log::debug(PSTR("[N2YO] GET... return payload: '%s'"), payload.c_str());

                    // {"info":{"satid":47947,"satname":"FEES","transactionscount":0,"passescount":2},"passes":[{"startAz":17.31,"startAzCompass":"NNE","startUTC":1630448555,"maxAz":98.2,"maxAzCompass":"E","maxEl":50.18,"maxUTC":1630448910,"endAz":181.59,"endAzCompass":"S","endUTC":1630449265},{"startAz":18.64,"startAzCompass":"NNE","startUTC":1630534640,"maxAz":99.66,"maxAzCompass":"E","maxEl":43.5,"maxUTC":1630534995,"endAz":178.42,"endAzCompass":"S","endUTC":1630535345}]}

                    DeserializationError error = deserializeJson(doc, payload);

                    if (error)
                    {
                        Log::debug(PSTR("[N2YO] deserializeJson() failed with code: %s"), error.c_str());
                    }
                    else
                    {
                        Log::debug(PSTR("[N2YO] deserializeJson() succeeded..."));

                        if (doc.containsKey("info"))
                        {
                            JsonVariant info = doc["info"];

                            if (info.containsKey("transactionscount")){
                                radiopass.transaction_count = info["transactionscount"].as<long>();
                                Log::debug(PSTR("[N2YO] transactionscount: %d"), radiopass.transaction_count);
                                }

                            if (info.containsKey("satid")){
                                radiopass.norad_id = info["satid"].as<long>();
                                Log::debug(PSTR("[N2YO] SAT id: %d"), radiopass.norad_id);
                                }

                            JsonVariant satname = info["satname"];
                            if (!satname.isNull()){
                                strncpy(radiopass.satname, satname.as<char *>(), MAXSATNAMELEN);
                                Log::debug(PSTR("[N2YO] SAT name: '%s'"), radiopass.satname);
                                }

                            if (info.containsKey("passescount")){
                                radiopass.passes_count = info["passescount"].as<int>();
                                Log::debug(PSTR("[N2YO] passescount: %d"), radiopass.passes_count);
                                }
                        }

                        if (doc.containsKey("passes"))
                        {
                            JsonArray array = doc["passes"].as<JsonArray>();

                            if (array.isNull())
                            {
                                Log::debug(PSTR("[N2YO] passes array is null"));
                            }
                            else
                            {
                                size_t passitems;

                                passitems = array.size();

                                Log::debug(PSTR("[N2YO] passes array has %d items..."), passitems);

                                time_t utc;
                                struct tm *currenttime;

                                time(&utc);
                                currenttime = gmtime(&utc);
                                Log::debug(PSTR("current UTC: %04d/%02d/%02d %02d:%02d:%02d (UTC)"), 1900 + currenttime->tm_year, currenttime->tm_mon, currenttime->tm_mday, currenttime->tm_hour, currenttime->tm_min, currenttime->tm_sec);

                                // timestamp is in UNIX format and is seconds; see https://www.epochconverter.com
                                for (int i = 0; i < passitems; i++)
                                {
                                    JsonVariant pass = array[i];
                                    if (!pass.containsKey("startUTC") || !pass.containsKey("endUTC") || !pass.containsKey("startAz") || !pass.containsKey("maxAz") || !pass.containsKey("maxEl") || !pass.containsKey("endAz"))
                                        break;

                                    time_t startutc = pass["startUTC"].as<long>();
                                    struct tm *passstarttime = gmtime(&startutc);

                                    time_t endutc = pass["endUTC"].as<long>();
                                    struct tm *passendtime = gmtime(&endutc);

                                    Log::debug(PSTR("[N2YO] pass #%03d: startUTC=%ld (%04d/%02d/%02d %02d:%02d:%02d) endUTC=%ld (%04d/%02d/%02d %02d:%02d:%02d) startAz=%f maxAz=%f maxEl=%f endAz=%f"), i,
                                               startutc, 1900 + passstarttime->tm_year, passstarttime->tm_mon, passstarttime->tm_mday, passstarttime->tm_hour, passstarttime->tm_min, passstarttime->tm_sec,
                                               endutc, 1900 + passendtime->tm_year, passendtime->tm_mon, passendtime->tm_mday, passendtime->tm_hour, passendtime->tm_min, passendtime->tm_sec,
                                               pass["startAz"].as<double>(), pass["maxAz"].as<double>(), pass["maxEl"].as<double>(), pass["endAz"].as<double>());
                                }
                            }
                        }
                    }
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

    // Add a scoping block for HTTPClient https to make sure it is destroyed before WiFiClientSecure *client is
    {
        HTTPClient https;

        Log::debug(PSTR("[N2YO] begin..."));

        // eg: https://api.n2yo.com/rest/v1/satellite/positions/47947/45.6989/9.67/150/5/&apiKey=XXXXXX-XXXXXX-XXXXXX-XXXX
        sprintf(buffer, "https://api.n2yo.com/rest/v1/satellite/positions/%u/%f/%f/%d/%d/&apiKey=%s", NORAD, lat, lon, alt, seconds, api_key);

        Log::debug(PSTR("[N2YO] current query is '%s'"), buffer);

        if (https.begin(*client, buffer))
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
                    String payload = https.getString();

                    Log::debug(PSTR("[N2YO] GET... return payload: '%s'"), payload.c_str());

                    // {"info":{"satname":"FEES","satid":47947,"transactionscount":0},"positions":[{"satlatitude":30.42417071,"satlongitude":144.83651694,"sataltitude":562.5,"azimuth":37.5,"elevation":-44.56,"ra":15.90889596,"dec":-6.24846802,"timestamp":1632922519,"eclipsed":true},{"satlatitude":30.36211626,"satlongitude":144.82133363,"sataltitude":562.48,"azimuth":37.54,"elevation":-44.59,"ra":15.8969196,"dec":-6.28436274,"timestamp":1632922520,"eclipsed":true},

                    DeserializationError error = deserializeJson(doc, payload);

                    if (error)
                    {
                        Log::debug(PSTR("[N2YO] deserializeJson() failed with code: %s"), error.c_str());
                    }
                    else
                    {
                        Log::debug(PSTR("[N2YO] deserializeJson() succeeded..."));

                        if (doc.containsKey("info"))
                        {
                            JsonVariant info = doc["info"];

                            if (info.containsKey("transactionscount"))
                                Log::debug(PSTR("[N2YO] transactionscount: %d"), info["transactionscount"].as<long>());

                            if (info.containsKey("satid"))
                                Log::debug(PSTR("[N2YO] SAT id: %d"), info["satid"].as<long>());

                            JsonVariant satname = info["satname"];
                            if (!satname.isNull())
                                Log::debug(PSTR("[N2YO] SAT name: '%s'"), satname.as<char *>());
                        }

                        if (doc.containsKey("positions"))
                        {
                            JsonArray array = doc["positions"].as<JsonArray>();

                            if (array.isNull())
                            {
                                Log::debug(PSTR("[N2YO] position array is null"));
                            }
                            else
                            {
                                size_t positems;

                                positems = array.size();

                                Log::debug(PSTR("[N2YO] position array has %d items..."), positems);

                                time_t utc;
                                struct tm *currenttime;

                                time(&utc);
                                currenttime = gmtime(&utc);
                                Log::debug(PSTR("current UTC: %04d/%02d/%02d %02d:%02d:%02d (UTC)"), 1900 + currenttime->tm_year, currenttime->tm_mon, currenttime->tm_mday, currenttime->tm_hour, currenttime->tm_min, currenttime->tm_sec);

                                // timestamp is in UNIX format and is seconds; see https://www.epochconverter.com
                                for (int i = 0; i < positems; i++)
                                {
                                    JsonVariant pos = array[i];
                                    if (!pos.containsKey("timestamp") || !pos.containsKey("azimuth") || !pos.containsKey("elevation"))
                                        break;
                                    time_t posutc = pos["timestamp"].as<long>();
                                    struct tm *postime = gmtime(&posutc);
                                    Log::debug(PSTR("[N2YO] pos #%03d: timestamp=%ld UTC:%04d/%02d/%02d %02d:%02d:%02d azimuth=%f elevation=%f"), i, posutc, 1900 + postime->tm_year, postime->tm_mon, postime->tm_mday, postime->tm_hour, postime->tm_min, postime->tm_sec, pos["azimuth"].as<double>(), pos["elevation"].as<double>());
                                }
                            }
                        }
                    }
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

    delete client;

    return true;
}
