#ifndef DEVICE_INFO_H
#define DEVICE_INFO_H

#include <Arduino.h>
#include "i2c_communicator.h"

// 设备信息结构体
struct DeviceInfo {
    int temperature;
    int muteThreshold;
    int stopThreshold;
    unsigned long runtime;
    String firmwareVersion;
    String parameterVersion;
    String dataVersion;
    String lotNumber;
    String serialNumber;
    bool infoValid;
    unsigned long lastUpdate;
    
    DeviceInfo() : temperature(-1), muteThreshold(0), stopThreshold(0), 
                   runtime(0), firmwareVersion("Unknown"), 
                   parameterVersion("Unknown"), dataVersion("Unknown"),
                   lotNumber("Unknown"), serialNumber("Unknown"),
                   infoValid(false), lastUpdate(0) {}
};

class DeviceInfoManager {
public:
    DeviceInfoManager();
    
    // 设置I2C通信器
    void setI2CCommunicator(I2CCommunicator* i2c);
    
    // 请求所有设备信息
    void requestAllInfo();
    
    // 请求温度信息
    bool requestTemperature();
    
    // 请求运行时间
    bool requestRuntime();
    
    // 请求版本信息
    bool requestVersion();
    
    // 请求LOT号
    bool requestLOTNumber();
    
    // 请求序列号
    bool requestSerialNumber();
    
    // 获取设备信息
    const DeviceInfo& getInfo() const;
    
    // 单个信息getter方法
    int getTemperature() const { return info.temperature; }
    int getMuteThreshold() const { return info.muteThreshold; }
    int getStopThreshold() const { return info.stopThreshold; }
    unsigned long getRuntime() const { return info.runtime; }
    String getFirmwareVersion() const { return info.firmwareVersion; }
    String getParameterVersion() const { return info.parameterVersion; }
    String getDataVersion() const { return info.dataVersion; }
    String getLotNumber() const { return info.lotNumber; }
    String getSerialNumber() const { return info.serialNumber; }
    
    // 更新温度信息（从notify回调）
    void updateTemperature(int temp, int mute, int stop);
    
    // 请求所有信息（别名）
    void requestAll() { requestAllInfo(); }
    
    // 检查信息是否过期
    bool isInfoExpired(unsigned long timeoutMs = 60000) const;
    
private:
    I2CCommunicator* i2cComm;
    DeviceInfo info;
};

#endif // DEVICE_INFO_H
