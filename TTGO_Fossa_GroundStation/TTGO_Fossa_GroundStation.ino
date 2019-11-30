/*
 
    Based on theRadioLib SX127x Receive with Interrupts Example
    For full API reference, see the GitHub Pages
     https://jgromes.github.io/RadioLib/


  */

// include the library
#include "config_manager.h"
#include <RadioLib.h>
#include <SPI.h>
#include "Comms.h"
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier

#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"       https://github.com/ThingPulse/esp8266-oled-ssd1306
#include "OLEDDisplayUi.h"                                  //    https://github.com/ThingPulse/esp8266-oled-ssd1306

#include "graphics.h"
#include <ArduinoJson.h>                                    //    https://github.com/bblanchon/ArduinoJson


#include <WiFi.h>
#include "time.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "esp32_mqtt_client.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <FS.h>
#include <SPIFFS.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>


Esp32_mqtt_clientClass mqtt;



//**********************
// User configuration //

const char* ssid              = ""; //your WiFi SSID
const char* password          = ""; //your Wifi Password
const char*  station          = "your_station_name"   ;
const float latitude          =  40.64 ;    // ** Beware this information is publically available use max 3 decimals 
const float longitude         =  -3.98 ;    // ** Beware this information is publically available use max 3 decimals 

#define MQTT_SERVER "fossa.apaluba.com"
#define MQTT_PORT 8883
#define MQTT_USER ""           // ask for user and password on the Telegram group
#define MQTT_PASS ""           // https://t.me/joinchat/DmYSElZahiJGwHX6jCzB3Q 

// Oled board configuration  uncomment your board
// SSD1306 display( address, OLED_SDA, OLED_SCL)
SSD1306 display(0x3c, 4, 15);         // configuration for TTGO v1 and Heltec
// SSD1306 display(0x3c, 21, 22);      // configuration for TTGO v2 (SMA antenna connector)
#define OLED_RST  16                   // seems that all board until now use pin 16

//*********************

const int fs_version =   1911303;      // version year month day 

OLEDDisplayUi ui     ( &display );








void manageMQTTEvent (esp_mqtt_event_id_t event) {
  Serial.printf ("MQTT event %d\n", event);
  if (event == MQTT_EVENT_CONNECTED) {
    char topic[64];
    strcpy(topic,station);
    strcat(topic,"/data/#");
    mqtt.subscribe (topic);
    Serial.println (topic);
    
  }
}

void manageMQTTData (char* topic, size_t topic_len, char* payload, size_t payload_len) {
  // Don't use Serial.print here. It will not work. Use ESP_LOG or printf instead.
  ESP_LOGI ("TEST","Received MQTT message: %.*s : %.*s", topic_len, topic, payload_len, payload);
  printf ("RRRRReceived MQTT message: %.*s : %.*s", topic_len, topic, payload_len, payload);
}







const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;         // 3600 for Spain
const int   daylightOffset_sec = 3600;


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}



//OLED pins to ESP32 TTGO v1 GPIOs 
//OLED_SDA — GPIO4
//OLED_SCL — GPIO15
//OLED_RST — GPIO16
// WIFI_LoRa_32 ports
// GPIO5 — SX1278’s SCK
// GPIO19 — SX1278’s MISO
// GPIO27 — SX1278’s MOSI
// GPIO18 — SX1278’s CS
// GPIO14 — SX1278’s RESET
// GPIO26 — SX1278’s IRQ(Interrupt Request)

// SX1278 has the following connections:
SX1278 lora = new Module(18, 26, 12);
// NSS pin:   18
// DIO0 pin:  26
// DIO1 pin:  12 (not used as isn't connected in the TTGo) 



#define LORA_CARRIER_FREQUENCY                          436.7f  // MHz
#define LORA_BANDWIDTH                                  125.0f  // kHz dual sideband
#define LORA_SPREADING_FACTOR                           11
#define LORA_SPREADING_FACTOR_ALT                       10
#define LORA_CODING_RATE                                8       // 4/8, Extended Hamming
#define LORA_OUTPUT_POWER                               21      // dBm
#define LORA_CURRENT_LIMIT                              120     // mA

// satellite callsign
char callsign[] = "FOSSASAT-1";


// Last packet received         "22:23:23"
String last_packet_received_time = " Waiting      ";
float last_packet_received_rssi;
float last_packet_received_snr;
float last_packet_received_frequencyerror;

// flag to indicate that a packet was received
volatile bool receivedFlag = false;

// disable interrupt when it's not needed
volatile bool enableInterrupt = true;

// this function is called when a complete packet
// is received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void) {
  // check if the interrupt is enabled
  if(!enableInterrupt) {
    return;
  }

  // we got a packet, set the flag
  receivedFlag = true;
}




/////// OLED ANIMATION //////

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
 // display->drawString(128, 0, String(millis()));

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

   // ********************* 
  // display time in digital format
  String thisTime="";
  if (timeinfo.tm_hour < 10){ thisTime=thisTime + " ";} // add leading space if required
  thisTime=String(timeinfo.tm_hour) + ":";
  if (timeinfo.tm_min < 10){ thisTime=thisTime + "0";} // add leading zero if required
  thisTime=thisTime + String(timeinfo.tm_min) + ":";
  if (timeinfo.tm_sec < 10){ thisTime=thisTime + "0";} // add leading zero if required
  thisTime=thisTime + String(timeinfo.tm_sec);
  const char* newTime = (const char*) thisTime.c_str();
  display->drawString(128, 0,  newTime  );


}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y

  display->drawXbm(x , y + 14, Fossa_Logo_width, Fossa_Logo_height, Fossa_Logo_bits);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString( x+70, y + 40, "Sta: "+ String(station));
  }


//Initial dummy System info:
float batteryChargingVoltage = 3.50;
float batteryChargingCurrent = 0.0204;
float batteryVoltage = 1.90;
float solarCellAVoltage = 0.96;
float solarCellBVoltage = 1.00;
float solarCellCVoltage = 1.06;
float batteryTemperature = 83.17;
float boardTemperature = 107.59;
int mcuTemperature = 42;
int resetCounter = 0;
byte powerConfig = 0b11111111;


void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demo for drawStringMaxWidth:
  // with the third parameter you can define the width after which words will be wrapped.
  // Currently only spaces and "-" are allowed for wrapping
 // display->setTextAlignment(TEXT_ALIGN_LEFT);
 // display->setFont(ArialMT_Plain_10);
 // display->drawStringMaxWidth(0 + x, 10 + y, 128, "Lorem ipsum\n dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore.");

  // draw an xbm image.
  // Please note that everything that should be transitioned
  // needs to be drawn relative to x and y
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawXbm(x + 34, y + 22, bat_width, bat_height, bat_bits);
  display->drawString( x+44, y + 10, String(batteryVoltage) + "V");
  display->drawString( x+13,  22+y,  String(batteryChargingVoltage));
  display->drawString( x+13,  35+y,  String(batteryChargingCurrent));
  display->drawString( x+80,  32+y,  String(batteryTemperature) + "ºC" );


}





void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x,  10+ y, "Solar panels:");
  //display->drawString( x,  12 +y, "Bat:" + String(batteryVoltage) + "V Ch:" + String(batteryChargingVoltage) + "V " + String(batteryChargingCurrent)+ "A"  );
  display->drawString( x,  21+y, "A:" + String(solarCellAVoltage) + "V  B:" + String(solarCellBVoltage) + "V  C:" + String(solarCellCVoltage)+ "V"  );
  display->drawString( x,  38+y, "T uC: " + String(boardTemperature) + "ºC   Reset: " + String(resetCounter)  );

}




void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x,  11+ y, "Last Packet: "+last_packet_received_time);
  display->drawString( x,  23+y, "RSSI:" + String(last_packet_received_rssi) + "dBm" );
  display->drawString( x,  34+y, "SNR: "+ String(last_packet_received_snr) +"dB" );
  display->drawString( x, 45+ y, "Freq error: " + String(last_packet_received_frequencyerror) +" Hz");
 
}





void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->drawXbm(x + 34, y + 4, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 42 + y, "Connected "+(WiFi.localIP().toString()));



/*  
 *   
  // Text alignment demo
  display->setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0 + x, 11 + y, "Left aligned (0,10)");

  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 22 + y, "Center aligned (64,22)");

  // The coordinates define the right end of the text
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->drawString(128 + x, 33 + y, "Right aligned (128,33)");

 */

}


//void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
//
//}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4, drawFrame5 };

// how many frames are there?
int frameCount = 5;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;





void setup() {

  pinMode(OLED_RST,OUTPUT);
  digitalWrite(OLED_RST, LOW);      //    Reset the OLED    
  delay(50);
  digitalWrite(OLED_RST, HIGH);


  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0,5,"FossaSAT-1 Sta");
  display.setFont(ArialMT_Plain_10);
  display.drawString(55,23,"ver. "+String(fs_version));
  
  display.drawString(5,38,"developed by");
  display.drawString(20,52,"@gmag12 & @g4lile0");
  display.display();

  delay (2000);  
  Serial.begin (115200);
  Serial.printf("Fossa Ground station Version to %d ", fs_version);

  display.clear();
  display.drawXbm(34, 0 , WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64 , 35 , "Connecting "+String(ssid));
  display.display();

  delay (500);  
  
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  uint8_t waiting = 0;
  while (WiFi.status() != WL_CONNECTED) {

      display.drawProgressBar(5,53,120,10,waiting ); 
      waiting += 10 ;
      if (waiting > 90) {
        waiting=0;
        display.setColor(BLACK);
      // display.drawProgressBar(5,53,120,10,100);   seems that doesn't seem to follow setColor
        display.fillRect(5, 53, 123, 12);
        display.setColor(WHITE);
        
      }
      display.display();
      delay(500);
      Serial.print(".");

  }

  display.clear();
  display.drawXbm(34, 0 , WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  
  Serial.println(" CONNECTED");
  display.drawString(64 , 35 , "Connected "+String(ssid));
  display.drawString(64 ,53 , (WiFi.localIP().toString()));
  display.display();
  delay (1000);  

  mqtt.init (MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS);

   char topic[64];
   strcpy(topic,station);
   strcat(topic,"/status");
   
  mqtt.setLastWill(topic);
  mqtt.onEvent (manageMQTTEvent);
  mqtt.onReceive (manageMQTTData);
  mqtt.begin ();
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  // The ESP is capable of rendering 60fps in 80Mhz mode
  // but that won't give you much time for anything else
  // run it in 160Mhz mode or just set it to 30 fps
  ui.setTargetFPS(60);

  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(BOTTOM);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  display.flipScreenVertically();

 
  // initialize SX1278 with default settings
  Serial.print(F("[SX1278] Initializing ... "));
  // carrier frequency:           436.7 MHz
  // bandwidth:                   125.0 kHz
  // spreading factor:            11
  // coding rate:                 8
  // sync word:                   0xff
  // output power:                17 dBm
  // current limit:               100 mA
  // preamble length:             8 symbols
  // amplifier gain:              0 (automatic gain control)
  //  int state = lora.begin();
 
     int state = lora.begin(LORA_CARRIER_FREQUENCY,
                            LORA_BANDWIDTH,
                            LORA_SPREADING_FACTOR,
                            LORA_CODING_RATE,
                            0xff,
                            17,
                            (uint8_t)LORA_CURRENT_LIMIT);
                            Serial.println(LORA_CARRIER_FREQUENCY);
                            Serial.println(LORA_SPREADING_FACTOR);
                            Serial.println(LORA_CODING_RATE);
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // set the function that will be called
  // when new packet is received
  lora.setDio0Action(setFlag);

  // start listening for LoRa packets
  Serial.print(F("[SX1278] Starting to listen ... "));
  state = lora.startReceive();
  if (state == ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // if needed, 'listen' mode can be disabled by calling
  // any of the following methods:
  //
  // lora.standby()
  // lora.sleep()
  // lora.transmit();
  // lora.receive();
  // lora.readData();
  // lora.scanChannel();


  welcome_message();

}







void loop() {


  int remainingTimeBudget = ui.update();

    if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }


  // check if the flag is set (received interruption)
  if(receivedFlag) {
    // disable the interrupt service routine while
    // processing the data
    enableInterrupt = false;

    // reset flag
    receivedFlag = false;




    // read received data
    size_t respLen = lora.getPacketLength();
    uint8_t* respFrame = new uint8_t[respLen];
    int state = lora.readData(respFrame, respLen);

    // get function ID
      uint8_t functionId = FCP_Get_FunctionID(callsign, respFrame, respLen);
      Serial.print(F("Function ID: 0x"));
      Serial.println(functionId, HEX);

      // check optional data
      uint8_t* respOptData = nullptr;
      uint8_t respOptDataLen = FCP_Get_OptData_Length(callsign, respFrame, respLen);
      Serial.print(F("Optional data ("));
      Serial.print(respOptDataLen);
      Serial.println(F(" bytes):"));
      if(respOptDataLen > 0) {
        // read optional data
        respOptData = new uint8_t[respOptDataLen];
        FCP_Get_OptData(callsign, respFrame, respLen, respOptData);
        PRINT_BUFF(respFrame, respLen);
      }



    struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

        // store time of the last packet received:
   String thisTime="";
  if (timeinfo.tm_hour < 10){ thisTime=thisTime + " ";} // add leading space if required
  thisTime=String(timeinfo.tm_hour) + ":";
  if (timeinfo.tm_min < 10){ thisTime=thisTime + "0";} // add leading zero if required
  thisTime=thisTime + String(timeinfo.tm_min) + ":";
  if (timeinfo.tm_sec < 10){ thisTime=thisTime + "0";} // add leading zero if required
  thisTime=thisTime + String(timeinfo.tm_sec);
 // const char* newTime = (const char*) thisTime.c_str();
    
    last_packet_received_time=  thisTime;
    last_packet_received_rssi= lora.getRSSI();
    last_packet_received_snr=lora.getSNR();
    last_packet_received_frequencyerror=lora.getFrequencyError();

      // print RSSI (Received Signal Strength Indicator)
      Serial.print(F("[SX1278] RSSI:\t\t"));
      Serial.print(lora.getRSSI());
      Serial.println(F(" dBm"));

      // print SNR (Signal-to-Noise Ratio)
      Serial.print(F("[SX1278] SNR:\t\t"));
      Serial.print(lora.getSNR());
      Serial.println(F(" dB"));

      // print frequency error
      Serial.print(F("[SX1278] Frequency error:\t"));
      Serial.print(lora.getFrequencyError());
      Serial.println(F(" Hz"));






      // process received frame
      switch(functionId) {
        case RESP_PONG:
          Serial.println(F("Pong!"));
          break;

        case RESP_SYSTEM_INFO:
          
          Serial.println(F("System info:"));

          Serial.print(F("batteryChargingVoltage = "));
          batteryChargingVoltage = FCP_Get_Battery_Charging_Voltage(respOptData);
          Serial.println(FCP_Get_Battery_Charging_Voltage(respOptData));
          
          Serial.print(F("batteryChargingCurrent = "));
          batteryChargingCurrent = (FCP_Get_Battery_Charging_Current(respOptData), 4);
          Serial.println(FCP_Get_Battery_Charging_Current(respOptData), 4);

          Serial.print(F("batteryVoltage = "));
          batteryVoltage=FCP_Get_Battery_Voltage(respOptData);
          Serial.println(FCP_Get_Battery_Voltage(respOptData));          

          Serial.print(F("solarCellAVoltage = "));
          solarCellAVoltage= FCP_Get_Solar_Cell_Voltage(0, respOptData);
          Serial.println(FCP_Get_Solar_Cell_Voltage(0, respOptData));

          Serial.print(F("solarCellBVoltage = "));
          solarCellBVoltage= FCP_Get_Solar_Cell_Voltage(1, respOptData);
          Serial.println(FCP_Get_Solar_Cell_Voltage(1, respOptData));

          Serial.print(F("solarCellCVoltage = "));
          solarCellCVoltage= FCP_Get_Solar_Cell_Voltage(2, respOptData);
          Serial.println(FCP_Get_Solar_Cell_Voltage(2, respOptData));

          Serial.print(F("batteryTemperature = "));
          batteryTemperature=FCP_Get_Battery_Temperature(respOptData);
          Serial.println(FCP_Get_Battery_Temperature(respOptData));

          Serial.print(F("boardTemperature = "));
          boardTemperature=FCP_Get_Board_Temperature(respOptData);
          Serial.println(FCP_Get_Board_Temperature(respOptData));

          Serial.print(F("mcuTemperature = "));
          mcuTemperature =FCP_Get_MCU_Temperature(respOptData);
          Serial.println(FCP_Get_MCU_Temperature(respOptData));

          Serial.print(F("resetCounter = "));
          resetCounter=FCP_Get_Reset_Counter(respOptData);
          Serial.println(FCP_Get_Reset_Counter(respOptData));

          Serial.print(F("powerConfig = 0b"));
          powerConfig=FCP_Get_Power_Configuration(respOptData);
          Serial.println(FCP_Get_Power_Configuration(respOptData), BIN);
          json_system_info();
          break;

        case RESP_LAST_PACKET_INFO:
          Serial.println(F("Last packet info:"));

          Serial.print(F("SNR = "));
          Serial.print(respOptData[0] / 4.0);
          Serial.println(F(" dB"));

          Serial.print(F("RSSI = "));
          Serial.print(respOptData[1] / -2.0);
          Serial.println(F(" dBm"));
          break;

        case RESP_REPEATED_MESSAGE:
          Serial.println(F("Got repeated message:"));
          Serial.println((char*)respOptData);
          break;

        default:
          Serial.println(F("Unknown function ID!"));
          break;
      }


      
    // you can also read received data as byte array
    /*
      byte byteArr[8];
      int state = lora.receive(byteArr, 8);
    */

    // you can read received data as an Arduino String
    //    String str;
    //    int state = lora.readData(str);

    if (state == ERR_NONE) {
      // packet was successfully received
  //     Serial.println(F("[SX1278] Received packet!"));

      // print data of the packet
 //     Serial.print(F("[SX1278] Data:\t\t"));
 //      Serial.println(str);



    } else if (state == ERR_CRC_MISMATCH) {
      // packet was received, but is malformed
      Serial.println(F("[SX1278] CRC error!"));

    } else {
      // some other error occurred
      Serial.print(F("[SX1278] Failed, code "));
      Serial.println(state);
    }

    // put module back to listen mode
    lora.startReceive();

    // we're ready to receive more packets,
    // enable interrupt service routine
    enableInterrupt = true;
  }

}


void  welcome_message (void) {




        const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(16);
          DynamicJsonDocument doc(capacity);
          doc["station"] = station;  // G4lile0
          JsonArray station_location = doc.createNestedArray("station_location");
          station_location.add(latitude);
          station_location.add(longitude);
          doc["version"] = fs_version;

          
//          doc["time"] = ;
          serializeJson(doc, Serial);
          char topic[64];
          strcpy(topic,station);
          strcat(topic,"/welcome");
          char buffer[512];
          serializeJson(doc, buffer);
          size_t n = serializeJson(doc, buffer);
          mqtt.publish(topic, buffer,n );

  
}




void  json_system_info(void) {
          //// JSON
          
          const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(16);
          DynamicJsonDocument doc(capacity);
          doc["station"] = station;  // G4lile0
          JsonArray station_location = doc.createNestedArray("station_location");
          station_location.add(latitude);
          station_location.add(longitude);
          doc["rssi"] = last_packet_received_rssi;
          doc["snr"] = last_packet_received_snr;
          doc["frequency_error"] = last_packet_received_frequencyerror;
          doc["batteryChargingVoltage"] = batteryChargingVoltage;
          doc["batteryChargingCurrent"] = batteryChargingCurrent;
          doc["batteryVoltage"] = batteryVoltage;
          doc["solarCellAVoltage"] = solarCellAVoltage;
          doc["solarCellBVoltage"] = solarCellBVoltage;
          doc["solarCellCVoltage"] = solarCellCVoltage;
          doc["batteryTemperature"] = batteryTemperature;
          doc["boardTemperature"] = boardTemperature;
          doc["mcuTemperature"] = mcuTemperature;
          doc["resetCounter"] = resetCounter;
          doc["powerConfig"] = powerConfig;
          serializeJson(doc, Serial);
          char topic[64];
          strcpy(topic,station);
          strcat(topic,"/sys_info");
          char buffer[512];
          serializeJson(doc, buffer);
          size_t n = serializeJson(doc, buffer);
          mqtt.publish(topic, buffer,n );


/*
 * 
 * 
 * https://arduinojson.org/v6/assistant/
 * {
"station":"g4lile0",
"station_location":[48.756080,2.302038],
"rssi":-34,
"snr": 9.50,
"frequency_error": 6406.27,
"batteryChargingVoltage": 3.46,
"batteryChargingCurrent":  -0.0110,
"batteryVoltage" : 1.96,
"solarCellAVoltage" : 0.92,
"solarCellBVoltage" : 0.96,
"solarCellCVoltage" : 0.96,
"batteryTemperature" : -26.71,
"boardTemperature" : 8.44,
"mcuTemperature" : 3,
"resetCounter" : 0,
"powerConfig" : 255

}
 */
 
}