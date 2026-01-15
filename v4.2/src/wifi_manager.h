#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWiFiManager.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class WiFiManager {
public:
    WiFiManager(AsyncWebServer& server);
    
    // 初始化WiFi
    void begin();
    
    // 启动AP模式
    void startAPMode();
    
    // 启动STA模式
    void startSTAMode(const String& ssid, const String& pwd);
    
    // 设置WiFi模式配置标志
    void setWifiConfigured(bool configured);
    bool isWifiConfigured() const;
    
    // 设置Tx Power
    void setTxPower(int powerLevel);
    
    // 开始WiFi扫描
    void startScan();
    
    // 处理WiFi扫描（在loop中调用）
    void processScan();
    
    // 检查WiFi重连和回退（在loop中调用）
    void checkReconnectFallback();
    
    // 是否正在扫描
    bool isScanning() const;
    
    // 是否等待WiFi连接
    bool isWaitingForWiFi() const;
    
    // 获取扫描结果
    String getScanResults() const;
    
    // 断开WiFi
    void disconnect();
    
    // 获取当前WiFi状态JSON
    String getWifiStatusJSON(uint8_t lang) const;
    
    // 获取WiFi状态JSON（别名）
    String getStatusJSON(uint8_t lang) const { return getWifiStatusJSON(lang); }
    
    // 请求扫描
    void requestScan();
    
private:
    AsyncWebServer& server;
    AsyncWiFiManager wifiManager;
    bool wifiConfigured;
    bool scanningWiFi;
    bool scanRequested;
    unsigned long scanStartTime;
    String wifiScanResults;
    bool waitingForWiFi;
    unsigned long connectStartTime;
    
    void enableMDNS();
};

#endif // WIFI_MANAGER_H
