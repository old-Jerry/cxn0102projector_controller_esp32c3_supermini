#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ---------------------- Pins & Device ----------------------
#define SDA_PIN        8
#define SCL_PIN        7
#define BUTTON_PIN     2
#define I2C_ADDRESS    0x77
#define COM_REQ_PIN    10  // GPIO10 用于 COM_REQ

// ---------------------- SoftAP ------------------------------
extern const char* AP_SSID;
extern const char* AP_PASSWORD;

// ---------------------- EEPROM Layout -----------------------
#define EEPROM_SIZE   64
#define ADDR_MAGIC    63
#define MAGIC_VALUE   0xA5

// Compact layout
#define ADDR_PAN      0   // int8_t   [-30,30]
#define ADDR_TILT     1   // int8_t   [-20,20]
#define ADDR_FLIP     2   // uint8_t  [0..3]
#define ADDR_TXPOWER  3   // int8_t   one of {78,76,74,68,60,52,44,34,28,20,8,-4}
#define ADDR_LANG     4   // uint8_t  0=en,1=zh

// EEPROM 地址扩展
#define ADDR_BRIGHTNESS  5
#define ADDR_CONTRAST    6
#define ADDR_HUE         7      // 旧:单一色调
#define ADDR_SATURATION  8      // 旧:单一饱和度
#define ADDR_SHARPNESS   9
#define ADDR_HUE_U       10     // 新增
#define ADDR_HUE_V       11     // 新增
#define ADDR_SAT_U       12     // 新增
#define ADDR_SAT_V       13     // 新增

#endif // CONFIG_H
