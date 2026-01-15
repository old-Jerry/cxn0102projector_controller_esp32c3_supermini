#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include <Arduino.h>

// 系统设置结构体
struct SystemSettings {
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
    String ssid;
    String pwd;
    uint8_t fanMode;
    bool wifiConfigured;
};

class EEPROMManager {
public:
    EEPROMManager();
    
    // 初始化 EEPROM
    void begin();
    
    // 加载设置
    void loadSettings(SystemSettings& settings);
    
    // 保存设置
    void saveSettings(const SystemSettings& settings);
    
    // 获取当前设置
    SystemSettings getSettings();
    
    // 清除所有设置
    void clearAll();
    
private:
    bool isValid;
    SystemSettings currentSettings;
};

#endif // EEPROM_MANAGER_H
