#ifndef I2C_COMMUNICATOR_H
#define I2C_COMMUNICATOR_H

#include <Arduino.h>
#include "eeprom_manager.h"

// 通知回调函数类型
typedef void (*NotifyCallback)(uint8_t cmd, uint8_t size, uint8_t result, const uint8_t* data, uint8_t length);

class I2CCommunicator {
public:
    I2CCommunicator();
    
    // 初始化 I2C
    void begin(NotifyCallback callback = nullptr);
    
    // 发送梯形校正和翻转
    void sendKeystoneAndFlip(int pan, int tilt, int flip);
    
    // 发送测试图案
    void sendTestPattern(uint8_t pattern, uint8_t generalSetting = 0x00,
                        uint8_t bgR = 0x00, uint8_t bgG = 0x00, uint8_t bgB = 0x00,
                        uint8_t fgR = 0xFF, uint8_t fgG = 0xFF, uint8_t fgB = 0xFF);
    
    // 发送图片质量设置
    void sendPictureQuality(const SystemSettings& settings);
    
    // 发送单个图片质量命令（用于亮度、对比度、锐度）
    void sendPictureQualityCommand(uint8_t cmd, uint8_t size, int8_t value);
    
    // 发送保存所有参数命令
    void sendSaveAll();
    
    // 发送保存所有参数命令（简化版本）
    void sendSaveAllCommand();
    
    // 发送恢复出厂设置命令
    void sendFactoryReset();
    
    // 发送恢复出厂设置命令（简化版本）
    void sendFactoryResetCommand();
    
    // 中断服务例程
    void handleCOM_REQ_ISR();
    
    // 处理通知（在loop中调用）
    void processNotify();
    
    // 请求温度信息
    bool requestTemperature(int& temperature, int& muteThreshold, int& stopThreshold);
    
    // 请求运行时间
    bool requestRuntime(unsigned long& runtime);
    
    // 请求版本信息
    bool requestVersion(String& firmware, String& parameter, String& data);
    
    // 请求LOT号
    bool requestLOTNumber(String& lotNumber);
    
    // 请求序列号
    bool requestSerialNumber(String& serialNumber);
    
private:
    NotifyCallback notifyCallback;
    volatile bool notifyPending;
    uint8_t notifyBuffer[32];
    uint8_t notifyLength;
    unsigned long lastNotifyTime;
    
    // 内部辅助函数
    bool sendInfoRequestAndRead(uint8_t cmd, uint8_t* response, uint8_t expectedLength);
    String parseVersion(const uint8_t* data, uint8_t startIndex);
    String parseLOTNumber(const uint8_t* data, uint8_t startIndex);
    String parseSerialNumber(const uint8_t* data, uint8_t startIndex);
};

#endif // I2C_COMMUNICATOR_H
