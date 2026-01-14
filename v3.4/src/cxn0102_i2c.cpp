#include "cxn0102_i2c.h"
#include "esp_log.h"
#include <string.h>

// Command strings array (0-indexed internally, but 1-indexed for API)
static const char* commands[] = {
    "0100",      // 1: Start Input
    "0200",      // 2: Stop Input
    "0b0101",    // 3: Reboot
    "0b0100",    // 4: Shutdown
    "3200",      // 5: Enter Optical Axis Adjustment
    "3300",      // 6: Optical Axis +
    "3400",      // 7: Optical Axis -
    "350100",    // 8: Exit Optical Axis (No Save)
    "350101",    // 9: Exit Optical Axis (Save)
    "3600",      // 10: Enter Bi-Phase Adjustment
    "3700",      // 11: Bi-Phase +
    "3800",      // 12: Bi-Phase -
    "390100",    // 13: Exit Bi-Phase (No Save)
    "390101",    // 14: Exit Bi-Phase (Save)
    "4A",        // 15: Flip Mode
    "5001",      // 16: Test Image ON
    "5000",      // 17: Test Image OFF
    "6000",      // 18: Mute
    "6001",      // 19: Unmute
    "7000",      // 20: Keystone Vertical -
    "7001",      // 21: Keystone Vertical +
    "7002",      // 22: Keystone Horizontal -
    "7003",      // 23: Keystone Horizontal +
    "8000",      // 24: Color Temperature -
    "8001",      // 25: Color Temperature +
    "4300",      // 26: Set Brightness (default value)
    "4500",      // 27: Set Contrast
    "4700",      // 28: Set Hue
    "4900",      // 29: Set Saturation
    "4F00"       // 30: Set Sharpness
};

CXN0102I2C::CXN0102I2C() : _initialized(false) {
}

void CXN0102I2C::init() {
    if (!_initialized) {
        Wire.begin(SDA_PIN, SCL_PIN);
        _initialized = true;
        Serial.println("[CXN0102I2C] Initialized.");
    }
}

void CXN0102I2C::sendI2CCommand(const char* cmd) {
    Wire.beginTransmission(I2C_ADDRESS);
    for (int i = 0; i < (int)strlen(cmd); i += 2) {
        char byteStr[3] = {cmd[i], cmd[i + 1], '\0'};
        uint8_t byteVal = (uint8_t)strtol(byteStr, NULL, 16);
        Wire.write(byteVal);
    }
    uint8_t error = Wire.endTransmission();
    if (error) {
        Serial.printf("[CXN0102I2C] I2C error: %d\n", error);
    } else {
        Serial.println("[CXN0102I2C] Command sent successfully.");
    }
}

void CXN0102I2C::sendCommand(int cmdIndex) {
    init();
    
    if (cmdIndex < 1 || cmdIndex > (int)(sizeof(commands) / sizeof(commands[0]))) {
        Serial.printf("[CXN0102I2C] Invalid command index: %d\n", cmdIndex);
        return;
    }
    
    sendI2CCommand(commands[cmdIndex - 1]);
}

void CXN0102I2C::sendCustomCommand(const char* cmd) {
    init();
    
    // Validation: must be even length hex, max 50 chars
    if (strlen(cmd) % 2 != 0 || strlen(cmd) > 50) {
        Serial.println("[CXN0102I2C] Invalid command format");
        return;
    }
    
    // Hex check
    for (size_t i = 0; i < strlen(cmd); ++i) {
        char c = cmd[i];
        bool ok = (c >= '0' && c <= '9') || (c>='a'&&c<='f') || (c>='A'&&c<='F');
        if (!ok) {
            Serial.println("[CXN0102I2C] Invalid hex content");
            return;
        }
    }
    
    sendI2CCommand(cmd);
}

void CXN0102I2C::sendKeystoneAndFlip(int pan, int tilt, int flip) {
    init();
    
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x26);  // Set Video Output Position Information
    Wire.write(0x09);  // Size
    Wire.write(pan & 0xFF);
    Wire.write(tilt & 0xFF);
    Wire.write(flip & 0xFF);
    for (int i = 0; i < 6; i++) Wire.write((i == 0) ? 0x64 : 0x00);  // Fixed values
    uint8_t error = Wire.endTransmission();
    if (error) {
        Serial.printf("[CXN0102I2C] I2C error while sending keystone command: %d\n", error);
    } else {
        Serial.println("[CXN0102I2C] Keystone and Flip command sent successfully.");
    }
}

void CXN0102I2C::setBrightness(uint8_t value) {
    init();
    
    int val = map(value, 0, 255, -31, 31);
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x43); // Set Brightness
    Wire.write(0x01);
    Wire.write((int8_t)val);
    Wire.endTransmission();
    Serial.printf("[CXN0102I2C] Set Brightness: %d (mapped to %d)\n", value, val);
}

void CXN0102I2C::setContrast(uint8_t value) {
    init();
    
    int val = map(value, 0, 255, -15, 15);
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x45); // Set Contrast
    Wire.write(0x01);
    Wire.write((int8_t)val);
    Wire.endTransmission();
    Serial.printf("[CXN0102I2C] Set Contrast: %d (mapped to %d)\n", value, val);
}

void CXN0102I2C::setHue(uint8_t u, uint8_t v) {
    init();
    
    int uVal = map(u, 0, 255, -15, 15);
    int vVal = map(v, 0, 255, -15, 15);
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x47); // Set Hue
    Wire.write(0x02); // OP0=2
    Wire.write((int8_t)uVal); // OP1: U
    Wire.write((int8_t)vVal); // OP2: V
    uint8_t error = Wire.endTransmission();
    Serial.printf("[CXN0102I2C] Set Hue U/V: %d/%d (mapped to %d/%d), I2C error=%d\n", 
                 u, v, uVal, vVal, error);
}

void CXN0102I2C::setSaturation(uint8_t u, uint8_t v) {
    init();
    
    int uVal = map(u, 0, 255, -15, 15);
    int vVal = map(v, 0, 255, -15, 15);
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x49); // Set Saturation
    Wire.write(0x02); // OP0=2
    Wire.write((int8_t)uVal); // OP1: U
    Wire.write((int8_t)vVal); // OP2: V
    Wire.endTransmission();
    Serial.printf("[CXN0102I2C] Set Saturation U/V: %d/%d (mapped to %d/%d)\n", 
                 u, v, uVal, vVal);
}

void CXN0102I2C::setSharpness(uint8_t value) {
    init();
    
    int val = map(value, 0, 255, 0, 8);
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x4F); // Set Sharpness
    Wire.write(0x01);
    Wire.write((uint8_t)val);
    Wire.endTransmission();
    Serial.printf("[CXN0102I2C] Set Sharpness: %d (mapped to %d)\n", value, val);
}

void CXN0102I2C::factoryReset() {
    init();
    
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x08); // 恢复出厂设置
    Wire.write(0x00);
    Wire.endTransmission();
    Serial.println("[CXN0102I2C] Factory reset command sent.");
}

void CXN0102I2C::saveAllParameters() {
    init();
    
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x07); // 保存所有参数
    Wire.write(0x05); // OP0
    Wire.write(0x00); // OP1
    Wire.write(0x00); // OP2
    Wire.write(0x01); // OP3:保存输出位置
    Wire.write(0x01); // OP4:保存光轴/双相位
    Wire.write(0x01); // OP5:保存画质信息
    Wire.endTransmission();
    Serial.println("[CXN0102I2C] Save all parameters command sent.");
}

void CXN0102I2C::sendTestPattern(int pattern) {
    init();
    
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0xA3); // 测试图案
    Wire.write(0x11); // OP0
    Wire.write((uint8_t)pattern); // OP1
    Wire.write(0x00); // OP2
    for (int i = 0; i < 15; ++i) Wire.write(0x00); // 填充剩余
    Wire.endTransmission();
    Serial.printf("[CXN0102I2C] Test pattern command sent: %d\n", pattern);
}

CXN0102I2C::TemperatureData CXN0102I2C::getTemperature() {
    init();
    
    TemperatureData data = {-1, -1, -1, -1};
    
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0xA0); // CMD
    Wire.write(0x00); // OP0
    Wire.endTransmission();

    delay(10); // Wait for device response

    // Read Notify response data (4 bytes: OP1, OP2, OP3, OP4)
    Wire.requestFrom(I2C_ADDRESS, 4);
    if (Wire.available() >= 4) {
        data.result = Wire.read();   // OP1
        data.temperature = Wire.read();   // OP2
        data.muteThreshold = Wire.read();   // OP3
        data.stopThreshold = Wire.read();   // OP4
        Serial.printf("[CXN0102I2C] Temperature: %d°C\n", data.temperature);
    }
    
    return data;
}

void CXN0102I2C::requestDeviceInfo() {
    init();
    
    // Send commands to request device info (actual reading needs separate I2C read/notify mechanism)
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0xA0); Wire.write(0x00); Wire.endTransmission(); // 温度
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0xA1); Wire.write(0x00); Wire.endTransmission(); // 运行时间
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0xA2); Wire.write(0x00); Wire.endTransmission(); // 版本
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0xB2); Wire.write(0x00); Wire.endTransmission(); // 批号
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0xB4); Wire.write(0x00); Wire.endTransmission(); // 序列号
    Serial.println("[CXN0102I2C] Device info commands sent.");
}
