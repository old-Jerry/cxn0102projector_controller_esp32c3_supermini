#include "eeprom_manager.h"
#include "config.h"
#include <EEPROM.h>

// 工具函数
static inline int clamp(int v, int lo, int hi) { 
    return v < lo ? lo : (v > hi ? hi : v); 
}

EEPROMManager::EEPROMManager() : isValid(false) {
}

void EEPROMManager::begin() {
    EEPROM.begin(EEPROM_SIZE);
    isValid = true;
}

void EEPROMManager::loadSettings(SystemSettings& settings) {
    if (!isValid) {
        Serial.println("[EEPROM] Not initialized!");
        return;
    }
    
    uint8_t magic = EEPROM.read(ADDR_MAGIC);
    if (magic != MAGIC_VALUE) {
        // First boot defaults
        settings.pan = DEFAULT_PAN;
        settings.tilt = DEFAULT_TILT;
        settings.flip = DEFAULT_FLIP;
        settings.txPower = DEFAULT_TXPOWER;
        settings.lang = DEFAULT_LANG;
        settings.brightness = DEFAULT_BRIGHTNESS;
        settings.contrast = DEFAULT_CONTRAST;
        settings.hue = DEFAULT_HUE;
        settings.saturation = DEFAULT_SATURATION;
        settings.sharpness = DEFAULT_SHARPNESS;
        settings.hueU = DEFAULT_HUE_U;
        settings.hueV = DEFAULT_HUE_V;
        settings.satU = DEFAULT_SAT_U;
        settings.satV = DEFAULT_SAT_V;
        //这里直接配置需要的SSID和PWD
        settings.ssid = "jerry_home";
        settings.pwd = "meiyijia";
        settings.fanMode = DEFAULT_FAN_MODE;
        settings.wifiConfigured = false;
        
        saveSettings(settings);
        Serial.println("[EEPROM] Initialized defaults.");
        return;
    }
    
    // 加载设置
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
    settings.wifiConfigured = EEPROM.read(ADDR_WIFI_FLAG) == 1;
    settings.fanMode = EEPROM.read(ADDR_FAN_MODE);
    
    // 加载SSID
    settings.ssid = "";
    for (int i = 0; i < 32; i++) {
        char c = EEPROM.read(ADDR_SSID + i);
        if (c == 0) break;
        settings.ssid += c;
    }
    
    // 加载PWD
    settings.pwd = "";
    for (int i = 0; i < 64; i++) {
        char c = EEPROM.read(ADDR_PWD + i);
        if (c == 0) break;
        settings.pwd += c;
    }
    
    // 数据验证和范围限制
    settings.pan = clamp(settings.pan, PAN_MIN, PAN_MAX);
    settings.tilt = clamp(settings.tilt, TILT_MIN, TILT_MAX);
    if (settings.flip < FLIP_MIN || settings.flip > FLIP_MAX) settings.flip = DEFAULT_FLIP;
    settings.brightness = clamp(settings.brightness, 0, 255);
    settings.contrast = clamp(settings.contrast, 0, 255);
    settings.hue = clamp(settings.hue, 0, 255);
    settings.saturation = clamp(settings.saturation, 0, 255);
    settings.sharpness = clamp(settings.sharpness, 0, 255);
    settings.hueU = clamp(settings.hueU, 0, 255);
    settings.hueV = clamp(settings.hueV, 0, 255);
    settings.satU = clamp(settings.satU, 0, 255);
    settings.satV = clamp(settings.satV, 0, 255);
    
    Serial.printf("[EEPROM] Loaded: pan=%d tilt=%d flip=%d txPower=%d lang=%u brightness=%u contrast=%u hue=%u hueU=%u hueV=%u saturation=%u satU=%u satV=%u sharpness=%u wifiConfigured=%d ssid=%s\n",
                  settings.pan, settings.tilt, settings.flip, settings.txPower, settings.lang, 
                  settings.brightness, settings.contrast, settings.hue, settings.hueU, settings.hueV, 
                  settings.saturation, settings.satU, settings.satV, settings.sharpness, 
                  settings.wifiConfigured, settings.ssid.c_str());
}

void EEPROMManager::saveSettings(const SystemSettings& settings) {
    if (!isValid) {
        Serial.println("[EEPROM] Not initialized!");
        return;
    }
    
    // 保存数值设置
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
    EEPROM.write(ADDR_WIFI_FLAG, settings.wifiConfigured ? 1 : 0);
    EEPROM.write(ADDR_FAN_MODE, settings.fanMode);
    
    // 保存SSID
    for (int i = 0; i < 32; i++) {
        EEPROM.write(ADDR_SSID + i, i < settings.ssid.length() ? settings.ssid[i] : 0);
    }
    
    // 保存PWD
    for (int i = 0; i < 64; i++) {
        EEPROM.write(ADDR_PWD + i, i < settings.pwd.length() ? settings.pwd[i] : 0);
    }
    
    EEPROM.write(ADDR_MAGIC, MAGIC_VALUE);
    EEPROM.commit();
    
    // 更新缓存
    currentSettings = settings;
    Serial.println("[EEPROM] Settings saved.");
}

SystemSettings EEPROMManager::getSettings() {
    return currentSettings;
}

void EEPROMManager::clearAll() {
    if (!isValid) {
        Serial.println("[EEPROM] Not initialized!");
        return;
    }
    
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    Serial.println("[EEPROM] All settings cleared.");
}
