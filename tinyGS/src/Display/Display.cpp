/*
  Display.cpp - Class responsible of controlling the display
  
  Copyright (C) 2020 -2021 @G4lile0, @gmag12 and @dev_4m1g0

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
OLEDDisplayUi* ui = NULL;

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame6(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame7(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
void drawFrame8(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);
// void drawFrame9(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y);


uint8_t frameCount = 8;
FrameCallback frames[] = { drawFrame1, drawFrame8, drawFrame3, drawFrame5, drawFrame2, drawFrame4, drawFrame6,drawFrame7 };
uint8_t overlaysCount = 1;
OverlayCallback overlays[] = { msOverlay };

unsigned long tick_interval;
int tick_timing = 100;
int graphVal = 1;
int delta = 1;
uint8_t oldOledBright = 100;

void displayInit()
{
  board_t board;
   if (!ConfigManager::getInstance().getBoardConfig(board))
    return;
  
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
  pinMode(board.OLED__RST,OUTPUT);
  digitalWrite(board.OLED__RST, LOW);     
  delay(50);
  digitalWrite(board.OLED__RST, HIGH);   
  display->init();

  if (ConfigManager::getInstance().getFlipOled())
    display->flipScreenVertically();
}

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state)
{
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);

  struct tm* timeinfo;
  time_t currenttime = time (NULL);
  if(currenttime < 0)
  {
    Serial.println("Failed to obtain time");
    return;
  }
  timeinfo = localtime (&currenttime);

  String thisTime="";
  if (timeinfo->tm_hour < 10){ thisTime=thisTime + " ";} // add leading space if required
  thisTime = String (timeinfo->tm_hour) + ":";
  if (timeinfo->tm_min < 10) { thisTime = thisTime + "0"; } // add leading zero if required
  thisTime = thisTime + String (timeinfo->tm_min) + ":";
  if (timeinfo->tm_sec < 10) { thisTime = thisTime + "0"; } // add leading zero if required
  thisTime = thisTime + String (timeinfo->tm_sec);
  const char* newTime = (const char*) thisTime.c_str();
  display->drawString(128, 0, newTime);

  if (ConfigManager::getInstance().getDayNightOled())
  {
    if (timeinfo->tm_hour < 6 || timeinfo->tm_hour > 18) display->normalDisplay(); else display->invertDisplay(); // change the OLED according to the time. 
  }


  if (oldOledBright!=ConfigManager::getInstance().getOledBright())
  {
    oldOledBright = ConfigManager::getInstance().getOledBright(); 
    if (ConfigManager::getInstance().getOledBright()==0) {
      display->displayOff();
    }
    else
    {
      display->setBrightness(2*ConfigManager::getInstance().getOledBright());
    }
  }
}

void drawRemoteFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y, uint8_t frameNumber)
{
  if (status.remoteTextFrameLength[frameNumber] == 0) ui->nextFrame();

  for (uint8_t n = 0; n < status.remoteTextFrameLength[frameNumber]; n++)
  {
    switch (status.remoteTextFrame[frameNumber][n].text_font)
    {
      case 2:
        display->setFont(ArialMT_Plain_16);
        break;
      default:
        display->setFont(ArialMT_Plain_10);
        break;
    }

    // 0 Left  1 Right  2 Center  3 Center Both
    switch (status.remoteTextFrame[frameNumber][n].text_alignment) {
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
    display->drawString(x+status.remoteTextFrame[frameNumber][n].text_pos_x, y+ status.remoteTextFrame[frameNumber][n].text_pos_y,  status.remoteTextFrame[frameNumber][n].text);
  }
}

void drawFrame1(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
  ConfigManager& configManager = ConfigManager::getInstance();

  display->drawXbm(x +10, y , Logo_width, Logo_height, Logo_bits);
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString( x+70, y + 32, "Sta: " + String(configManager.getThingName()));
}

void drawFrame2(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
  drawRemoteFrame(display, state, x, y, 0);
}

void drawFrame3(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
  if (!status.radio_ready)
  {
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->setFont(ArialMT_Plain_10);
    display->drawString(0, 0, "LoRa initialization failed.");
    display->drawString(0, 14, "Browse " + WiFi.localIP().toString());
    display->drawString(0, 28, "Ensure board selected");
    display->drawString(0, 42, "matches your hardware");

    return;
  }

  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(x,  y,  status.modeminfo.satellite);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64+ x,  12 + y,  String(status.modeminfo.modem_mode) + " @ " + String(status.modeminfo.frequency) + "MHz");
  //display->drawString(x,  12 + y, "F:" );
  //display->setTextAlignment(TEXT_ALIGN_RIGHT);
  
  display->setTextAlignment(TEXT_ALIGN_LEFT);

  if (String(status.modeminfo.modem_mode)=="LoRa")
  {
    display->drawString(x,  23 + y, "SF: " + String(status.modeminfo.sf));
    if (ConfigManager::getInstance().getAllowTx())
    {
      display->drawString(x,  34 + y, "Pwr:"+ String(status.modeminfo.power) + "dBm"); 
    }
    else
    {
      display->drawString(x,  34 + y, "TX OFF"); 
    }
    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->drawString(128 + x,  23 + y, "BW:"+ String(status.modeminfo.bw)+ "kHz");
    display->drawString(128 + x,  34 + y, "CR: "+ String(status.modeminfo.cr));
  } 
  else
  {
    display->drawString(x,  23 + y, "FD/BW: " );
    if (ConfigManager::getInstance().getAllowTx()) {
      display->drawString(x,  34 + y, "P:"+ String(status.modeminfo.power) + "dBm"); 
    }
    else
    {
      display->drawString(x,  34 + y, "TX OFF"); 
    }
    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->drawString(128 + x,  23 + y, String(status.modeminfo.freqDev)+ "/" + String(status.modeminfo.bw)+ "kHz");
    display->drawString(128 + x,  34 + y, String(status.modeminfo.bitrate)+ "kbps");
  }
  if (status.vbat != 0.0)
  {
      display->setTextAlignment(TEXT_ALIGN_LEFT);
      display->drawString(x, 45 + y, "Bat: " + String(status.vbat) + "V");
  }
}

void drawFrame4(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
  drawRemoteFrame(display, state, x, y, 1);
}

void drawFrame5(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
  display->drawXbm(x , y , earth_width, earth_height, earth_bits);
  display->setColor(BLACK);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->fillRect(83,0,128,11);
  display->setFont(ArialMT_Plain_10);
 
  if (status.satPos[0] == 0 && status.satPos[1] == 0)
  {
    String msg = F("Waiting for Sat Pos");
    display->drawString( 65+x,  49+y+(x/2), msg );
    display->drawString( 63+x,  51+y+(x/2), msg );
    display->setColor(WHITE);
    display->drawString( 64+x,  50+y+(x/2), msg );
  }
  else 
  {
    if ((millis()-tick_interval)>tick_timing)
    {
      // Change the value to plot
      graphVal+=delta;
      tick_interval=millis();
      // If the value reaches a limit, then change delta of value
      if (graphVal >= 6)      { delta = -1;  tick_timing=50; }// ramp down value
      else if (graphVal <= 1) { delta = +1; tick_timing=100; } // ramp up value
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

void drawFrame6(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
  drawRemoteFrame(display, state, x, y, 2);
}

void drawFrame7(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
  drawRemoteFrame(display, state, x, y, 3);
}

void drawFrame8(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(x+2, 16+y, "MQTT:" );
  if (status.mqtt_connected) { display->drawString( x+7,  26+y, "ON"); }  else { display->drawString( x+5,  26+y, "OFF"); }
  display->drawXbm(x + 32, y + 4, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  // The coordinates define the center of the text
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 + x, 42 + y, "Connected " + (WiFi.localIP().toString()));
}  

void displayShowConnected()
{
  display->clear();
  display->drawXbm(34, 0 , WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 , 35 , "Connected " + String(ConfigManager::getInstance().getWiFiSSID()));
  display->drawString(64 ,53 , (WiFi.localIP().toString()));
  display->display();
}

void displayShowInitialCredits()
{
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0,5,"tinyGS");
  display->setFont(ArialMT_Plain_10);
  display->drawString(50,23,"ver. " + String(status.version));

  display->drawString(5,38,"by @gmag12 @4m1g0");
  display->drawString(40,52,"& @g4lile0");
  display->display();
}

void displayShowApMode()
{
  display->clear();
  display->setFont(ArialMT_Plain_10);
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->drawString(0, 6,"Connect to AP:");
  display->drawString(0,18,"->"+String(ConfigManager::getInstance().getThingName()));
  display->drawString(5,32,"to configure your Station");
  display->drawString(10,52,"IP:   192.168.4.1");
  display->display();
}

void displayShowStaMode(bool ap)
{
  display->clear();
  display->drawXbm(34, 0 , WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->drawString(64 , 35 , "Connecting " + String(ConfigManager::getInstance().getWiFiSSID()));
  if (ap)
    display->drawString(64 , 52 , "Config AP available");
  display->display();
}

void displayUpdate()
{
  if (ConfigManager::getInstance().getOledBright())
    ui->update();
}

void displayTurnOff()
{
  display->displayOff();
}

void displayNextFrame() {
  if (ui)
    ui->nextFrame();
}
