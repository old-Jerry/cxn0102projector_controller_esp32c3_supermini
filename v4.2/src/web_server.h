#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>
#include "config.h"
#include "eeprom_manager.h"
#include "command_handler.h"
#include "i2c_communicator.h"
#include "device_info.h"
#include "fan_controller.h"
#include "wifi_manager.h"

class WebServer {
public:
    WebServer(AsyncWebServer& server,
              EEPROMManager& eepromMgr,
              CommandHandler& cmdHandler,
              I2CCommunicator& i2cComm,
              DeviceInfoManager& devInfoMgr,
              FanController& fanCtrl,
              WiFiManager& wifiMgr);
    
    void begin();
    
private:
    AsyncWebServer& server;
    EEPROMManager& eepromMgr;
    CommandHandler& cmdHandler;
    I2CCommunicator& i2cComm;
    DeviceInfoManager& devInfoMgr;
    FanController& fanCtrl;
    WiFiManager& wifiMgr;
    
    void setupRoutes();
    
    // Route handlers
    void handleRoot(AsyncWebServerRequest* request);
    void handleCommand(AsyncWebServerRequest* request);
    void handleKeystone(AsyncWebServerRequest* request);
    void handleCustomCommand(AsyncWebServerRequest* request);
    void handleTestPattern(AsyncWebServerRequest* request);
    void handleSetTxPower(AsyncWebServerRequest* request);
    void handlePing(AsyncWebServerRequest* request);
    void handleGetSettings(AsyncWebServerRequest* request);
    void handleSetSettings(AsyncWebServerRequest* request);
    void handleSetLang(AsyncWebServerRequest* request);
    void handleSetPQ(AsyncWebServerRequest* request);
    void handleFactoryReset(AsyncWebServerRequest* request);
    void handleSaveAll(AsyncWebServerRequest* request);
    void handleGetDeviceInfo(AsyncWebServerRequest* request);
    void handleGetTemperature(AsyncWebServerRequest* request);
    void handleGetNotifications(AsyncWebServerRequest* request);
    void handleClearEEPROM(AsyncWebServerRequest* request);
    void handleReboot(AsyncWebServerRequest* request);
    void handleWiFiScan(AsyncWebServerRequest* request);
    void handleWiFiList(AsyncWebServerRequest* request);
    void handleSetWiFiMode(AsyncWebServerRequest* request);
    void handleWiFiConnect(AsyncWebServerRequest* request);
    void handleWiFiDisconnect(AsyncWebServerRequest* request);
    void handleWiFiStatus(AsyncWebServerRequest* request);
    void handleSetFan(AsyncWebServerRequest* request);
};

#endif // WEB_SERVER_H
