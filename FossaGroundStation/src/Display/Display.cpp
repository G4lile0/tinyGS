/*
  Display.cpp - Class responsible of controlling the display
  
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

#include "Display.h"
#include "graphics.h"

SSD1306* display;
OLEDDisplayUi* ui;

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame6(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame7(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame8(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame9(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);


uint8_t frameCount = 9;
FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4, drawFrame5, drawFrame6,drawFrame7, drawFrame8,drawFrame9 };
uint8_t overlaysCount = 1;
OverlayCallback overlays[] = { msOverlay };

unsigned long tick_interval;
int tick_timing = 100;
int graphVal = 1;
int delta = 1;

void displayInit(){
  board_type board = ConfigManager::getInstance().getBoardConfig();
  display = new SSD1306(board.OLED__address, board.OLED__SDA, board.OLED__SCL);

  ui = new OLEDDisplayUi(display);
  ui->setTargetFPS(60);
  ui->setActiveSymbol(activeSymbol);
  ui->setInactiveSymbol(inactiveSymbol);
  ui->setIndicatorPosition(BOTTOM);
  ui->setIndicatorDirection(LEFT_RIGHT);
  ui->setFrameAnimation(SLIDE_LEFT);
  ui->setFrames(frames, frameCount);
  ui->setOverlays(overlays, overlaysCount);
  ui->init();
  display->flipScreenVertically();

  pinMode(board.OLED__RST,OUTPUT);
  digitalWrite(board.OLED__RST, LOW);     
  delay(50);
  digitalWrite(board.OLED__RST, HIGH);
}

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);

  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

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
  display->drawXbm(x , y + 6, Fossa_Logo_width, Fossa_Logo_height, Fossa_Logo_bits);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString( x+70, y + 42, "Sta: "+ String(ConfigManager::getInstance().getThingName()));
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x,  y,  status.modeminfo.satelite);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64+ x,  12 + y,  String(status.modeminfo.modem_mode) + " @ " + String(status.modeminfo.frequency) + "MHz" );
//  display->drawString(x,  12 + y, "F:" );
//  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  
  display->setTextAlignment(TEXT_ALIGN_LEFT);

  if (String(status.modeminfo.modem_mode)=="LoRa") {
      display->drawString(x,  23 + y, "SF: " + String(status.modeminfo.sf) );
      display->drawString( x,  34 + y, "Pwr:"+ String(status.modeminfo.power) + "dBm" );
      display->setTextAlignment(TEXT_ALIGN_RIGHT);
      display->drawString(128+x,  23 + y,"BW:"+ String(status.modeminfo.bw)+ "kHz" );
      display->drawString(128+x,  34 + y, "CR: "+ String(status.modeminfo.cr) );

  } else {
      display->drawString(x,  23 + y, "FD/BW: " );
      display->drawString( x,  34 + y, "P:"+ String(status.modeminfo.power) + "dBm" );
      display->setTextAlignment(TEXT_ALIGN_RIGHT);
      display->drawString(128+x,  23 + y, String(status.modeminfo.freqDev)+ "/" + String(status.modeminfo.rxBw)+ "kHz" );
      display->drawString(128+x,  34 + y, String(status.modeminfo.bitrate)+ "kbps");

  }
}



void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
//  display->setTextAlignment(TEXT_ALIGN_LEFT);
//  display->setFont(ArialMT_Plain_10);
//  display->drawString(x,  y,  "pantalla global");

  if (status.global_frame_text_leght==0 ) ui->nextFrame();

 for (uint8_t n=0; n<status.global_frame_text_leght;n++){

   switch (status.global_frame_text[n].text_font) {
     case 2:
      display->setFont(ArialMT_Plain_16);
      break;

      default:
      display->setFont(ArialMT_Plain_10);
      break;
   }

// 0 Left  1 Right  2 Center  3 Center Both
   switch (status.global_frame_text[n].text_alignment) {
     case 1:
      display->setTextAlignment(TEXT_ALIGN_RIGHT);
      break;
     case 2:
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      break;
     case 3:
      display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
      break;

      default:
      display->setTextAlignment(TEXT_ALIGN_LEFT);
      break;
   }
  display->drawString(x+status.global_frame_text[n].text_pos_x, y+ status.global_frame_text[n].text_pos_y,  String(status.global_frame_text[n].text));

       }

}

void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x,  y,  status.modeminfo.satelite);
  display->drawString(0 + x,  11 + y, "Last Packet: " );
  display->drawString(64 + x,  11 + y, status.lastPacketInfo.time);

  display->drawString( x,  23 + y, "RSSI: " );
  display->drawString( 64+ x,  23 + y,  String(status.lastPacketInfo.rssi) + "dBm" );

  display->drawString( x,  34 + y, "SNR:  " );
  display->drawString( 64 +x,  34 + y,  String(status.lastPacketInfo.snr) + "dB" );

  display->drawString( x, 45 + y, "Freq error: " );
  display->drawString( 64 + x, 45 + y,  String(status.lastPacketInfo.frequencyerror) + " Hz");
}

void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->drawXbm(x , y , earth_width, earth_height, earth_bits);
  display->setColor(BLACK);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->fillRect(83,0,128,11);
  display->setFont(ArialMT_Plain_10);
 
  if (status.satPos[0] == 0 && status.satPos[1] == 0) {
    String msg = F("Waiting for FossaSat Pos");
    display->drawString( 65+x,  49+y+(x/2), msg );
    display->drawString( 63+x,  51+y+(x/2), msg );
    display->setColor(WHITE);
    display->drawString( 64+x,  50+y+(x/2), msg );
  }
  else {
    if ((millis()-tick_interval)>tick_timing) {
      // Change the value to plot
      graphVal+=delta;
      tick_interval=millis();
      // If the value reaches a limit, then change delta of value
      if (graphVal >= 6)     {delta = -1;  tick_timing=50; }// ramp down value
      else if (graphVal <= 1) {delta = +1; tick_timing=100;} // ramp up value
    }
    display->fillCircle(status.satPos[0]+x, status.satPos[1]+y, graphVal+1);
    display->setColor(WHITE);
    display->drawCircle(status.satPos[0]+x, status.satPos[1]+y, graphVal);
    display->setColor(BLACK);
    display->drawCircle(status.satPos[0]+x, status.satPos[1]+y, (graphVal/3)+1);
    display->setColor(WHITE);
    display->drawCircle(status.satPos[0]+x, status.satPos[1]+y, graphVal/3);
  }
}

void drawFrame6(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x,  10 + y, "Solar panels:");
  display->drawString( x,  21 + y, "A:" + String(status.sysInfo.solarCellAVoltage) + "V  B:" + String(status.sysInfo.solarCellBVoltage) + "V  C:" + String(status.sysInfo.solarCellCVoltage)+ "V"  );
  display->drawString( x,  38 + y, "T uC: " + String(status.sysInfo.boardTemperature) + "ºC   Reset: " + String(status.sysInfo.resetCounter)  );

  if ((millis()-tick_interval)>200) {
    graphVal-=1;
    tick_interval=millis();
    if (graphVal <= 1) {graphVal = 8; } // ramp up value
    if ((status.sysInfo.batteryVoltage == 0) &&  (status.sysInfo.batteryChargingVoltage ==0) && (status.sysInfo.batteryTemperature == 0))   ui->nextFrame();  // if not data received move to the next frame
  }
}

void drawFrame7(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawXbm(x + 34, y + 22, bat_width, bat_height, bat_bits);
  display->drawString( x+44, y + 10, String(status.sysInfo.batteryVoltage) + "V");
  display->drawString( x+13,  22+y,  String(status.sysInfo.batteryChargingVoltage));
  display->drawString( x+13,  35+y,  String(status.sysInfo.batteryChargingCurrent));
  display->drawString( x+80,  32+y,  String(status.sysInfo.batteryTemperature) + "ºC" );


  if ((millis()-tick_interval)>200) {
    // Change the value to plot
    graphVal-=1;
    tick_interval=millis();
    if (graphVal <= 1) {graphVal = 8; } // ramp up value
    if ((status.sysInfo.batteryVoltage == 0) &&  (status.sysInfo.batteryChargingVoltage ==0) && (status.sysInfo.batteryTemperature == 0))   ui->nextFrame();  // if not data received move to the next frame
  }

  display->fillRect(x+48, y+32+graphVal, 25 , 13-graphVal);
}


void drawFrame8(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
//  display->setTextAlignment(TEXT_ALIGN_LEFT);
//  display->setFont(ArialMT_Plain_10);
//  display->drawString(x,  y,  "pantalla rec local");


  if (status.local_frame_text_leght==0 ) ui->nextFrame();

 for (uint8_t n=0; n<status.local_frame_text_leght;n++){
   switch (status.local_frame_text[n].text_font) {
     case 2:
      display->setFont(ArialMT_Plain_16);
      break;

      default:
      display->setFont(ArialMT_Plain_10);
      break;
   }

// 0 Left  1 Right  2 Center  3 Center Both
   switch (status.local_frame_text[n].text_alignment) {
     case 1:
      display->setTextAlignment(TEXT_ALIGN_RIGHT);
      break;
     case 2:
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      break;
     case 3:
      display->setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
      break;

      default:
      display->setTextAlignment(TEXT_ALIGN_LEFT);
      break;
   }

  display->drawString(x+status.local_frame_text[n].text_pos_x, y+ status.local_frame_text[n].text_pos_y,  status.local_frame_text[n].text);
       }




}


void drawFrame9(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString( x+100,  21+y, "MQTT:" );
  if (status.mqtt_connected ) {display->drawString( x+105,  31+y, "ON" );}  else {display->drawString( x+102,  31+y, "OFF" );}
  display->drawXbm(x + 34, y + 4, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 42 + y, "Connected "+(WiFi.localIP().toString()));
}  

void displayShowConnected() {
  display->clear();
  display->drawXbm(34, 0 , WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  
  Serial.println(" CONNECTED");
  display->drawString(64 , 35 , "Connected " + String(ConfigManager::getInstance().getWiFiSSID()));
  display->drawString(64 ,53 , (WiFi.localIP().toString()));
  display->display();
}

void displayShowWaitingMqttConnection() {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(20 ,10 , "Waiting for MQTT"); 
  display->drawString(35 ,24 , "Connection...");
  display->drawString(3 ,38 , "Press PROG butt. or send");
  display->drawString(3 ,52 , "e in serial to reset conf.");
  display->display();
}

void displayShowInitialCredits() {
  display->init();
  display->flipScreenVertically();
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0,5,"FossaSAT-1 Sta");
  display->setFont(ArialMT_Plain_10);
  display->drawString(55,23,"ver. " + String(status.version));

  display->drawString(5,38,"by @gmag12 @4m1g0");
  display->drawString(40,52,"& @g4lile0");
  display->display();
}

void displayShowApMode() {
  display->clear();
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 6,"Connect to AP:");
  display->drawString(0,18,"->"+String(ConfigManager::getInstance().getThingName()));
  display->drawString(5,32,"to configure your Station");
  display->drawString(10,52,"IP:   192.168.4.1");
  display->display();
}

void displayShowStaMode() {
  display->clear();
  display->drawXbm(34, 0 , WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 , 35 , "Connecting " + String(ConfigManager::getInstance().getWiFiSSID()));
  display->display();
}

void displayShowLoRaError() {
  display->clear();
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 0, "LoRa initialization failed.");
  display->drawString(0, 14, "Browse " + WiFi.localIP().toString());
  display->drawString(0, 28, "Ensure board selected");
  display->drawString(0, 42, "matches your hardware");
  display->display();
}

void displayUpdate() {
  ui->update();
  // NOTE: After some investigation I don't think it's necesary to manage time budget here, 
  // the update methos will skip frames when necesary without stoping the main thread.

  /*int remainingTimeBudget = ui->update();
  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }*/
}