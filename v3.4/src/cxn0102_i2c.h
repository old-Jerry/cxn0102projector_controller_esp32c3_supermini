#ifndef CXN0102_I2C_H
#define CXN0102_I2C_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// Command indices (1-based for compatibility with original)
enum CXNCommand {
    CMD_START_INPUT = 1,
    CMD_STOP_INPUT = 2,
    CMD_REBOOT = 3,
    CMD_SHUTDOWN = 4,
    CMD_OPTICAL_AXIS_ENTER = 5,
    CMD_OPTICAL_AXIS_PLUS = 6,
    CMD_OPTICAL_AXIS_MINUS = 7,
    CMD_OPTICAL_AXIS_EXIT_NO_SAVE = 8,
    CMD_OPTICAL_AXIS_EXIT_SAVE = 9,
    CMD_BIPHASE_ENTER = 10,
    CMD_BIPHASE_PLUS = 11,
    CMD_BIPHASE_MINUS = 12,
    CMD_BIPHASE_EXIT_NO_SAVE = 13,
    CMD_BIPHASE_EXIT_SAVE = 14,
    CMD_FLIP_MODE = 15,
    CMD_TEST_IMAGE_ON = 16,
    CMD_TEST_IMAGE_OFF = 17,
    CMD_MUTE = 18,
    CMD_UNMUTE = 19,
    CMD_KEYSTONE_VERTICAL_MINUS = 20,
    CMD_KEYSTONE_VERTICAL_PLUS = 21,
    CMD_KEYSTONE_HORIZONTAL_MINUS = 22,
    CMD_KEYSTONE_HORIZONTAL_PLUS = 23,
    CMD_COLOR_TEMP_MINUS = 24,
    CMD_COLOR_TEMP_PLUS = 25,
    CMD_SET_BRIGHTNESS = 26,
    CMD_SET_CONTRAST = 27,
    CMD_SET_HUE = 28,
    CMD_SET_SATURATION = 29,
    CMD_SET_SHARPNESS = 30
};

class CXN0102I2C {
public:
    CXN0102I2C();
    void init();
    
    // Command functions
    void sendCommand(int cmdIndex);
    void sendCustomCommand(const char* cmd);
    
    // Keystone and Flip
    void sendKeystoneAndFlip(int pan, int tilt, int flip);
    
    // Picture Quality
    void setBrightness(uint8_t value);  // 0-255 maps to -31 to 31
    void setContrast(uint8_t value);    // 0-255 maps to -15 to 15
    void setHue(uint8_t u, uint8_t v);  // 0-255 maps to -15 to 15
    void setSaturation(uint8_t u, uint8_t v); // 0-255 maps to -15 to 15
    void setSharpness(uint8_t value);   // 0-255 maps to 0 to 8
    
    // System functions
    void factoryReset();
    void saveAllParameters();
    void sendTestPattern(int pattern); // 0=stop, 1=color bar, 2=grid
    
    // Device info
    struct TemperatureData {
        int result;
        int temperature;
        int muteThreshold;
        int stopThreshold;
    };
    TemperatureData getTemperature();
    
    void requestDeviceInfo();
    
private:
    bool _initialized;
    void sendI2CCommand(const char* cmd);
    static inline int map(int x, int in_min, int in_max, int out_min, int out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }
};

#endif // CXN0102_I2C_H
