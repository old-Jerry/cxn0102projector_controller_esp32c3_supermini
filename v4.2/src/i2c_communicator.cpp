#include "i2c_communicator.h"
#include "config.h"
#include <Wire.h>

// 全局中断处理函数（用于attachInterrupt）
I2CCommunicator* g_i2cCommunicator = nullptr;

void IRAM_ATTR globalCOM_REQ_ISR() {
    if (g_i2cCommunicator) {
        g_i2cCommunicator->handleCOM_REQ_ISR();
    }
}

I2CCommunicator::I2CCommunicator() 
    : notifyCallback(nullptr), notifyPending(false), notifyLength(0), lastNotifyTime(0) {
}

void I2CCommunicator::begin(NotifyCallback callback) {
    notifyCallback = callback;
    g_i2cCommunicator = this;
    
    pinMode(COM_REQ_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(COM_REQ_PIN), globalCOM_REQ_ISR, RISING);
    Wire.begin(SDA_PIN, SCL_PIN);
    
    Serial.println("[I2C] Initialized");
}

void I2CCommunicator::sendKeystoneAndFlip(int pan, int tilt, int flip) {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x26); // Set Video Output Position Information
    Wire.write(0x09); // Size
    Wire.write(pan & 0xFF);
    Wire.write(tilt & 0xFF);
    Wire.write(flip & 0xFF);
    for (int i = 0; i < 6; i++) Wire.write((i == 0) ? 0x64 : 0x00); // Fixed values
    uint8_t error = Wire.endTransmission();
    if (error) {
        Serial.printf("[I2C] Error sending keystone command: %d\n", error);
    } else {
        Serial.println("[I2C] Keystone and Flip command sent successfully.");
    }
}

void I2CCommunicator::sendTestPattern(uint8_t pattern, uint8_t generalSetting,
                                        uint8_t bgR, uint8_t bgG, uint8_t bgB,
                                        uint8_t fgR, uint8_t fgG, uint8_t fgB) {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0xA3); // Output Test Picture
    Wire.write(0x11); // OP0: Size = 17 bytes
    Wire.write(pattern); // OP1: Test pattern
    Wire.write(generalSetting); // OP2: General purpose setting
    Wire.write(bgR); // OP3: Background color R
    Wire.write(bgG); // OP4: Background color G
    Wire.write(bgB); // OP5: Background color B
    Wire.write(fgR); // OP6: Foreground color R
    Wire.write(fgG); // OP7: Foreground color G
    Wire.write(fgB); // OP8: Foreground color B
    for (int i = 0; i < 9; i++) {
        Wire.write(0x00); // OP9-OP17: Reserved
    }
    uint8_t error = Wire.endTransmission();
    if (error) {
        Serial.printf("[I2C] Error sending test pattern: %d\n", error);
    } else {
        Serial.printf("[I2C] Test pattern sent: 0x%02X\n", pattern);
    }
}

void I2CCommunicator::sendPictureQuality(const SystemSettings& settings) {
    // Brightness
    int val = map(settings.brightness, 0, 255, -31, 31);
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x43); // Set Brightness
    Wire.write(0x01);
    Wire.write((int8_t)val);
    Wire.endTransmission();
    
    // Contrast
    val = map(settings.contrast, 0, 255, -15, 15);
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x45); // Set Contrast
    Wire.write(0x01);
    Wire.write((int8_t)val);
    Wire.endTransmission();
    
    // Hue (U/V)
    int u = map(settings.hueU, 0, 255, -15, 15);
    int v = map(settings.hueV, 0, 255, -15, 15);
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x47); // Set Hue
    Wire.write(0x02); // OP0=2
    Wire.write((int8_t)u); // OP1: U
    Wire.write((int8_t)v); // OP2: V
    Wire.endTransmission();
    
    // Saturation (U/V)
    u = map(settings.satU, 0, 255, -15, 15);
    v = map(settings.satV, 0, 255, -15, 15);
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x49); // Set Saturation
    Wire.write(0x02); // OP0=2
    Wire.write((int8_t)u); // OP1: U
    Wire.write((int8_t)v); // OP2: V
    Wire.endTransmission();
    
    // Sharpness
    val = map(settings.sharpness, 0, 255, 0, 8);
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x4F); // Set Sharpness
    Wire.write(0x01);
    Wire.write((uint8_t)val);
    Wire.endTransmission();
    
    Serial.println("[I2C] Picture quality settings sent");
}

void I2CCommunicator::sendPictureQualityCommand(uint8_t cmd, uint8_t size, int8_t value) {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(cmd);
    Wire.write(size);
    Wire.write(value);
    uint8_t error = Wire.endTransmission();
    if (error) {
        Serial.printf("[I2C] Error sending picture quality command 0x%02X: %d\n", cmd, error);
    }
}

void I2CCommunicator::sendSaveAll() {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x07); // 保存所有参数
    Wire.write(0x05); // OP0
    Wire.write(0x00); // OP1
    Wire.write(0x00); // OP2
    Wire.write(0x01); // OP3:保存输出位置
    Wire.write(0x01); // OP4:保存光轴/双相位
    Wire.write(0x01); // OP5:保存画质信息
    Wire.endTransmission();
    Serial.println("[I2C] Save all command sent");
}

void I2CCommunicator::sendFactoryReset() {
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(0x08); // 恢复出厂设置
    Wire.write(0x00);
    Wire.endTransmission();
    Serial.println("[I2C] Factory reset command sent");
}

void I2CCommunicator::sendSaveAllCommand() {
    sendSaveAll();
}

void I2CCommunicator::sendFactoryResetCommand() {
    sendFactoryReset();
}

void I2CCommunicator::handleCOM_REQ_ISR() {
    notifyPending = true;
    lastNotifyTime = millis();
}

void I2CCommunicator::processNotify() {
    if (!notifyPending) return;
    
    // 读取 Notify 数据
    Wire.requestFrom((uint8_t)I2C_ADDRESS, (uint8_t)32);
    notifyLength = 0;
    while (Wire.available() && notifyLength < 32) {
        notifyBuffer[notifyLength++] = Wire.read();
    }
    
    // 处理 Notify 数据
    if (notifyLength >= 3) {
        uint8_t cmd = notifyBuffer[0];
        uint8_t size = notifyBuffer[1];
        uint8_t result = notifyBuffer[2];
        
        Serial.printf("[NOTIFY] CMD: 0x%02X, Size: %d, Result: 0x%02X, Data: ", cmd, size, result);
        for (int i = 0; i < notifyLength; i++) {
            Serial.printf("%02X ", notifyBuffer[i]);
        }
        Serial.println();
        
        // 调用回调函数
        if (notifyCallback) {
            notifyCallback(cmd, size, result, notifyBuffer, notifyLength);
        }
        
        switch (cmd) {
            case 0x00: // Boot Completed Notify
                Serial.println("[NOTIFY] Boot Completed");
                if (result != 0x00) {
                    Serial.printf("[NOTIFY] Boot error: 0x%02X\n", result);
                }
                break;
            
            case 0x10: // Emergency Notify
                Serial.printf("[NOTIFY] Emergency: 0x%02X\n", result);
                break;
            
            case 0x11: // Temperature Emergency and Recovery Notify
                if (result == 0x80 || result == 0x81) {
                    Serial.printf("[NOTIFY] Temperature Emergency: 0x%02X\n", result);
                } else {
                    Serial.printf("[NOTIFY] Temperature Recovery: 0x%02X\n", result);
                }
                break;
            
            case 0x12: // Command Emergency Notify
                Serial.printf("[NOTIFY] Command Error: 0x%02X\n", result);
                break;
            
            default:
                Serial.printf("[NOTIFY] Unknown command: 0x%02X\n", cmd);
                break;
        }
    } else {
        Serial.println("[NOTIFY] Invalid notify data length");
    }
    
    notifyPending = false;
}

bool I2CCommunicator::sendInfoRequestAndRead(uint8_t cmd, uint8_t* response, uint8_t expectedLength) {
    // Send request
    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(cmd);
    Wire.write(0x00); // OP0=0
    uint8_t error = Wire.endTransmission();
    if (error) {
        Serial.printf("[I2C] Error sending request 0x%02X: %d\n", cmd, error);
        return false;
    }
    
    // Wait for response
    delay(50);
    
    // Read response
    Wire.requestFrom((uint8_t)I2C_ADDRESS, expectedLength);
    uint8_t readLength = 0;
    while (Wire.available() && readLength < expectedLength) {
        response[readLength++] = Wire.read();
    }
    
    if (readLength != expectedLength) {
        Serial.printf("[I2C] Incomplete response for 0x%02X: expected %d, got %d\n", cmd, expectedLength, readLength);
        return false;
    }
    
    // Check CMD and Result
    if (response[0] != cmd || response[2] != 0x00) {
        Serial.printf("[I2C] Invalid response for 0x%02X: CMD=0x%02X Result=0x%02X\n", cmd, response[0], response[2]);
        return false;
    }
    
    return true;
}

bool I2CCommunicator::requestTemperature(int& temperature, int& muteThreshold, int& stopThreshold) {
    uint8_t response[6];
    if (!sendInfoRequestAndRead(0xA0, response, 6)) {
        return false;
    }
    
    temperature = response[3];
    muteThreshold = response[4];
    stopThreshold = response[5];
    
    Serial.printf("[I2C] Temperature: %d°C, Mute: %d°C, Stop: %d°C\n",
                 temperature, muteThreshold, stopThreshold);
    return true;
}

bool I2CCommunicator::requestRuntime(unsigned long& runtime) {
    uint8_t response[7];
    if (!sendInfoRequestAndRead(0xA1, response, 7)) {
        return false;
    }
    
    runtime = ((unsigned long)response[6] << 24) |
               ((unsigned long)response[5] << 16) |
               ((unsigned long)response[4] << 8) |
               response[3];
    
    Serial.printf("[I2C] Runtime: %lu seconds (%lu hours %lu minutes)\n",
                 runtime, runtime/3600, (runtime%3600)/60);
    return true;
}

String I2CCommunicator::parseVersion(const uint8_t* data, uint8_t startIndex) {
    String version = "";
    for (int i = 0; i < 4; i++) {
        if (data[startIndex + i] >= 32 && data[startIndex + i] <= 126) {
            version += (char)data[startIndex + i];
        } else {
            char hex[3];
            sprintf(hex, "%02X", data[startIndex + i]);
            version += hex;
        }
    }
    return version;
}

String I2CCommunicator::parseLOTNumber(const uint8_t* data, uint8_t startIndex) {
    String lot = "";
    for (int lotIndex = 0; lotIndex < 3; lotIndex++) {
        int base = startIndex + (lotIndex * 4);
        char buffer[9];
        sprintf(buffer, "%02X%02X%02X%02X",
                data[base + 3], data[base + 2], data[base + 1], data[base]);
        lot += String(buffer);
        if (lotIndex < 2) lot += "-";
    }
    return lot;
}

String I2CCommunicator::parseSerialNumber(const uint8_t* data, uint8_t startIndex) {
    String serial = "";
    for (int serialIndex = 0; serialIndex < 2; serialIndex++) {
        int base = startIndex + (serialIndex * 4);
        char buffer[9];
        sprintf(buffer, "%02X%02X%02X%02X",
                data[base + 3], data[base + 2], data[base + 1], data[base]);
        serial += String(buffer);
        if (serialIndex < 1) serial += "-";
    }
    return serial;
}

bool I2CCommunicator::requestVersion(String& firmware, String& parameter, String& data) {
    uint8_t response[14];
    if (!sendInfoRequestAndRead(0xA2, response, 14)) {
        return false;
    }
    
    firmware = parseVersion(response, 3);
    parameter = parseVersion(response, 7);
    data = parseVersion(response, 11);
    
    Serial.printf("[I2C] Firmware: %s, Parameter: %s, Data: %s\n",
                 firmware.c_str(), parameter.c_str(), data.c_str());
    return true;
}

bool I2CCommunicator::requestLOTNumber(String& lotNumber) {
    uint8_t response[15];
    if (!sendInfoRequestAndRead(0xB2, response, 15)) {
        return false;
    }
    
    lotNumber = parseLOTNumber(response, 3);
    Serial.printf("[I2C] LOT Number: %s\n", lotNumber.c_str());
    return true;
}

bool I2CCommunicator::requestSerialNumber(String& serialNumber) {
    uint8_t response[11];
    if (!sendInfoRequestAndRead(0xB4, response, 11)) {
        return false;
    }
    
    serialNumber = parseSerialNumber(response, 3);
    Serial.printf("[I2C] Serial Number: %s\n", serialNumber.c_str());
    return true;
}
