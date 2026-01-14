#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

class WiFiManager {
public:
    WiFiManager();
    void init();
    void setTxPower(int powerLevel);  // Power level code: 78,76,74,68,60,52,44,34,28,20,8,-4
    float getTxPowerdBm(int powerLevel);  // Convert power code to dBm value
    
private:
    bool _initialized;
};

#endif // WIFI_MANAGER_H
