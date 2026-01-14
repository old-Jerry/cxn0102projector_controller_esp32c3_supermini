#include "eeprom_handler.h"
#include "esp_log.h"

EEPROMHandler::EEPROMHandler() : _initialized(false) {
}

void EEPROMHandler::init() {
    if (!_initialized) {
        EEPROM.begin(EEPROM_SIZE);
        _initialized = true;
        Serial.println("[EEPROM] Initialized.");
    }
}

void EEPROMHandler::loadSettings(Settings& settings) {
    init();
    
    uint8_t magic = EEPROM.read(ADDR_MAGIC);
    if (magic != MAGIC_VALUE) {
        // First boot defaults
        settings.pan = 0;
        settings.tilt = 0;
        settings.flip = 0;
        settings.txPower = 34;
        settings.lang = 0;
        settings.brightness = 128;
        settings.contrast = 128;
        settings.hue = 128;
        settings.saturation = 128;
        settings.sharpness = 128;
        settings.hueU = 128;
        settings.hueV = 128;
        settings.satU = 128;
        settings.satV = 128;
        
        saveSettings(settings);
        Serial.println("[EEPROM] Initialized defaults.");
        return;
    }
    
    settings.pan = (int8_t)EEPROM.read(ADDR_PAN);
    settings.tilt = (int8_t)EEPROM.read(ADDR_TILT);
    settings.flip = (uint8_t)EEPROM.read(ADDR_FLIP);
    settings.txPower = (int8_t)EEPROM.read(ADDR_TXPOWER);
    settings.lang = (uint8_t)EEPROM.read(ADDR_LANG);
    settings.brightness = EEPROM.read(ADDR_BRIGHTNESS);
    settings.contrast = EEPROM.read(ADDR_CONTRAST);
    settings.hue = EEPROM.read(ADDR_HUE);
    settings.saturation = EEPROM.read(ADDR_SATURATION);
    settings.sharpness = EEPROM.read(ADDR_SHARPNESS);
    settings.hueU = EEPROM.read(ADDR_HUE_U);
    settings.hueV = EEPROM.read(ADDR_HUE_V);
    settings.satU = EEPROM.read(ADDR_SAT_U);
    settings.satV = EEPROM.read(ADDR_SAT_V);

    // Basic sanitization
    settings.pan = clamp(settings.pan, -30, 30);
    settings.tilt = clamp(settings.tilt, -20, 20);
    if (settings.flip < 0 || settings.flip > 3) settings.flip = 0;
    settings.brightness = clamp(settings.brightness, 0, 255);
    settings.contrast = clamp(settings.contrast, 0, 255);
    settings.hue = clamp(settings.hue, 0, 255);
    settings.saturation = clamp(settings.saturation, 0, 255);
    settings.sharpness = clamp(settings.sharpness, 0, 255);
    settings.hueU = clamp(settings.hueU, 0, 255);
    settings.hueV = clamp(settings.hueV, 0, 255);
    settings.satU = clamp(settings.satU, 0, 255);
    settings.satV = clamp(settings.satV, 0, 255);

    Serial.printf("[EEPROM] Loaded: pan=%d tilt=%d flip=%d txPower=%d lang=%u brightness=%u contrast=%u hue=%u hueU=%u hueV=%u saturation=%u satU=%u satV=%u sharpness=%u\n",
                  settings.pan, settings.tilt, settings.flip, settings.txPower, settings.lang,
                  settings.brightness, settings.contrast, settings.hue, settings.hueU, settings.hueV,
                  settings.saturation, settings.satU, settings.satV, settings.sharpness);
}

void EEPROMHandler::saveSettings(const Settings& settings) {
    init();
    
    EEPROM.write(ADDR_PAN, (int8_t)settings.pan);
    EEPROM.write(ADDR_TILT, (int8_t)settings.tilt);
    EEPROM.write(ADDR_FLIP, (uint8_t)settings.flip);
    EEPROM.write(ADDR_TXPOWER, (int8_t)settings.txPower);
    EEPROM.write(ADDR_LANG, (uint8_t)settings.lang);
    EEPROM.write(ADDR_BRIGHTNESS, settings.brightness);
    EEPROM.write(ADDR_CONTRAST, settings.contrast);
    EEPROM.write(ADDR_HUE, settings.hue);
    EEPROM.write(ADDR_SATURATION, settings.saturation);
    EEPROM.write(ADDR_SHARPNESS, settings.sharpness);
    EEPROM.write(ADDR_HUE_U, settings.hueU);
    EEPROM.write(ADDR_HUE_V, settings.hueV);
    EEPROM.write(ADDR_SAT_U, settings.satU);
    EEPROM.write(ADDR_SAT_V, settings.satV);
    EEPROM.write(ADDR_MAGIC, MAGIC_VALUE);
    EEPROM.commit();
    Serial.println("[EEPROM] Settings saved.");
}

void EEPROMHandler::clearEEPROM() {
    init();
    
    // Clear all bytes
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    Serial.println("[EEPROM] Cleared.");
}
