#ifndef CONFIG_H
#define CONFIG_H

// ---------------------- Pins & Device ----------------------
#define SDA_PIN 8
#define SCL_PIN 7
#define BUTTON_PIN 2
#define I2C_ADDRESS 0x77
#define COM_REQ_PIN 10 // GPIO10 用于 COM_REQ

// ---------------------- Fan PWM -----------------------------
#define FAN_PWM_PIN 12
#define FAN_PWM_CHANNEL 0
#define FAN_PWM_FREQ 25000     // 25kHz 静音风扇常用频率
#define FAN_PWM_RES 8          // 0~255

// ---------------------- SoftAP ------------------------------
const char* AP_SSID = "CXN0102_Web_Controller";
const char* AP_PASSWORD = "12345678"; // 必须 ≥ 8 字符

// ---------------------- EEPROM Layout -----------------------
#define EEPROM_SIZE 128 // 扩展到128以存储SSID(32)和PWD(64)
#define ADDR_MAGIC 127
#define MAGIC_VALUE 0xA5

// Compact layout
#define ADDR_PAN 0 // int8_t [-30,30]
#define ADDR_TILT 1 // int8_t [-20,20]
#define ADDR_FLIP 2 // uint8_t [0..3]
#define ADDR_TXPOWER 3 // int8_t one of {78,76,74,68,60,52,44,34,28,20,8,-4}
#define ADDR_LANG 4 // uint8_t 0=en,1=zh

// EEPROM 地址扩展
#define ADDR_BRIGHTNESS 5
#define ADDR_CONTRAST 6
#define ADDR_HUE 7 // 旧:单一色调
#define ADDR_SATURATION 8 // 旧:单一饱和度
#define ADDR_SHARPNESS 9
#define ADDR_HUE_U 10 // 新增
#define ADDR_HUE_V 11 // 新增
#define ADDR_SAT_U 12 // 新增
#define ADDR_SAT_V 13 // 新增

// 新增：SSID (32 bytes)
#define ADDR_SSID 14
// 新增：PWD (64 bytes)
#define ADDR_PWD 46

// 新增：WiFi 配网启用标志地址（0=AP only,1=enable STA）
#define ADDR_WIFI_FLAG 110

#define ADDR_FAN_MODE 120   // uint8_t fanMode 

// ---------------------- WiFi Scan --------------------------
const unsigned long SCAN_TIMEOUT = 10000; // 10秒扫描超时

// ---------------------- Timers ---------------------------
#define WIFI_CHECK_INTERVAL 10000      // 10秒检查一次WiFi
#define DEVICE_INFO_UPDATE_INTERVAL 60000  // 60秒更新一次设备信息
#define FAN_CHECK_INTERVAL 60000       // 60秒检查一次风扇

// ---------------------- Default Values ----------------------
#define DEFAULT_PAN 0
#define DEFAULT_TILT 0
#define DEFAULT_FLIP 0
#define DEFAULT_TXPOWER 34 // 8.5dBm
#define DEFAULT_LANG 1     // zh
#define DEFAULT_BRIGHTNESS 128
#define DEFAULT_CONTRAST 128
#define DEFAULT_HUE 128
#define DEFAULT_SATURATION 128
#define DEFAULT_SHARPNESS 128
#define DEFAULT_HUE_U 128
#define DEFAULT_HUE_V 128
#define DEFAULT_SAT_U 128
#define DEFAULT_SAT_V 128
#define DEFAULT_FAN_MODE 3 // Auto

// ---------------------- Value Ranges ----------------------
#define PAN_MIN -30
#define PAN_MAX 30
#define TILT_MIN -20
#define TILT_MAX 20
#define FLIP_MIN 0
#define FLIP_MAX 3

#endif // CONFIG_H
