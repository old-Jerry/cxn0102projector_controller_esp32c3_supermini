#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "eeprom_handler.h"
#include "cxn0102_i2c.h"

class WebServerHandler {
public:
    WebServerHandler(CXN0102I2C& cxn, EEPROMHandler& eeprom);
    void init();
    void updateSettings(const Settings& settings);
    
private:
    AsyncWebServer _server;
    CXN0102I2C& _cxn;
    EEPROMHandler& _eeprom;
    Settings _settings;
    
    static inline int clamp(int v, int lo, int hi) { 
        return v < lo ? lo : (v > hi ? hi : v); 
    }
    
    void setupRoutes();
    String buildMainPageHtml();
};

#endif // WEB_SERVER_H
