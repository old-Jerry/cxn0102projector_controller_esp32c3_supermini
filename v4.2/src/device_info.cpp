#include "device_info.h"

DeviceInfoManager::DeviceInfoManager() : i2cComm(nullptr) {
}

void DeviceInfoManager::setI2CCommunicator(I2CCommunicator* i2c) {
    i2cComm = i2c;
}

void DeviceInfoManager::requestAllInfo() {
    if (!i2cComm) {
        Serial.println("[DEVICE] I2C communicator not set!");
        return;
    }
    
    Serial.println("[DEVICE] ===== Requesting all device information =====");
    
    // 依次请求所有信息，增加延迟避免冲突
    if (requestTemperature()) {
        delay(400);
    }
    if (requestRuntime()) {
        delay(400);
    }
    if (requestVersion()) {
        delay(400);
    }
    if (requestLOTNumber()) {
        delay(400);
    }
    if (requestSerialNumber()) {
        delay(400);
    }
    
    info.infoValid = true;
    info.lastUpdate = millis();
    
    Serial.println("[DEVICE] ===== All device info requests completed =====");
}

bool DeviceInfoManager::requestTemperature() {
    if (!i2cComm) return false;
    
    bool success = i2cComm->requestTemperature(info.temperature, 
                                                info.muteThreshold, 
                                                info.stopThreshold);
    if (success) {
        info.lastUpdate = millis();
    }
    return success;
}

bool DeviceInfoManager::requestRuntime() {
    if (!i2cComm) return false;
    
    bool success = i2cComm->requestRuntime(info.runtime);
    if (success) {
        info.lastUpdate = millis();
    }
    return success;
}

bool DeviceInfoManager::requestVersion() {
    if (!i2cComm) return false;
    
    bool success = i2cComm->requestVersion(info.firmwareVersion,
                                            info.parameterVersion,
                                            info.dataVersion);
    if (success) {
        info.lastUpdate = millis();
    }
    return success;
}

bool DeviceInfoManager::requestLOTNumber() {
    if (!i2cComm) return false;
    
    bool success = i2cComm->requestLOTNumber(info.lotNumber);
    if (success) {
        info.lastUpdate = millis();
    }
    return success;
}

bool DeviceInfoManager::requestSerialNumber() {
    if (!i2cComm) return false;
    
    bool success = i2cComm->requestSerialNumber(info.serialNumber);
    if (success) {
        info.lastUpdate = millis();
    }
    return success;
}

const DeviceInfo& DeviceInfoManager::getInfo() const {
    return info;
}

void DeviceInfoManager::updateTemperature(int temp, int mute, int stop) {
    info.temperature = temp;
    info.muteThreshold = mute;
    info.stopThreshold = stop;
    info.lastUpdate = millis();
    info.infoValid = true;
}

bool DeviceInfoManager::isInfoExpired(unsigned long timeoutMs) const {
    return !info.infoValid || (millis() - info.lastUpdate > timeoutMs);
}
