#ifndef EEPROM_HANDLER_H
#define EEPROM_HANDLER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"

// Settings structure
struct Settings {
    int pan;
    int tilt;
    int flip;
    int txPower;
    uint8_t lang;
    uint8_t brightness;
    uint8_t contrast;
    uint8_t hue;
    uint8_t saturation;
    uint8_t sharpness;
    uint8_t hueU;
    uint8_t hueV;
    uint8_t satU;
    uint8_t satV;
};

class EEPROMHandler {
public:
    EEPROMHandler();
    void init();
    void loadSettings(Settings& settings);
    void saveSettings(const Settings& settings);
    void clearEEPROM();
    
private:
    bool _initialized;
    static inline int clamp(int v, int lo, int hi) { 
        return v < lo ? lo : (v > hi ? hi : v); 
    }
};

#endif // EEPROM_HANDLER_H
