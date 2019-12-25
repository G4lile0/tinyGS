
// uncomment the line matching your board by removing the //

#define TTGO_V1
//#define TTGO_V2
//#define HELTEC
//# No specification in config file: try autodetection (gpio4 pin level at startup)

// Working in a new automated config board.

/*

# OLED Setup is depending on hardware of LoRa board
# TTGO v1:   SDA=4   SCL=15, RST=16 
# TTGO v2:   SDA=21  SCL=22, RST=16       
# T-BEAM :   SDA=21, SCL=22, RST=16
# HELTEC 2:  SDA=4   SCL=15, RST=16 

    OLED_address, OLED_SDA, OLED_SCL, OLED_RST , PROG_BUTTON, BOARD_LED,  

*/

typedef struct {
   uint8_t  OLED__address;
   uint8_t  OLED__SDA;
   uint8_t  OLED__SCL;
   uint8_t  OLED__RST;
   uint8_t  PROG__BUTTON;
   uint8_t  BOARD_LED;
   bool     L_SX1278;
   uint8_t  L_NSS;        // CS
   uint8_t  L_DI00;        
   uint8_t  L_DI01;
   uint8_t  L_DI02;        
   uint8_t  L_RST;
   uint8_t  L_MISO;
   uint8_t  L_MOSI;        
   uint8_t  L_SCK;
   String   BOARD;
} board_type;

board_type  boards[] {
 //OLED_add, OLED_SDA,  OLED_SCL, OLED_RST, PROG_BUTTON, BOARD_LED, L_SX1278?, L_NSS, L_DI00, L_DI01, L_DI02, L_RST,  L_MISO, L_MOSI, L_SCK, BOARD 
{      0x3c,        4,        15,       16,           0,        25,      true,    18,     33,     32,      0,    14,      19,     27,     5, "HELTEC WiFi LoRA 32 V1" },
{      0x3c,        4,        15,       16,           0,        25,      true,    18,     35,     34,      0,    14,      19,     27,     5, "HELTEC WiFi LoRA 32 V2" }, 
{      0x3c,        4,        15,       16,           0,         2,      true,    18,     26,      0,      0,    14,      19,     27,     5 ,"TTGO LoRa 32 v1"        },  
{      0x3c,       21,        22,       16,           0,        22,      true,    18,     26,      0,      0,    14,      19,     27,     5 ,"TTGO LoRA 32 v2"        }, // 3  ((SMA antenna connector))
{      0x3c,       21,        22,       16,          39,        22,      true,    18,     26,     33,     32,    14,      19,     27,     5 ,"T-BEAM + OLED  "        }, 
{      0x3c,       21,        22,       16,           0,        25,     false,     2,     26,      0,      0,    14,      19,     27,     5 ,"Custom ESP32 + SX126x"  },   
};
