#include "web_server.h"
#include <SPIFFS.h>
#include <string.h>

WebServer::WebServer(AsyncWebServer& server,
                     EEPROMManager& eepromMgr,
                     CommandHandler& cmdHandler,
                     I2CCommunicator& i2cComm,
                     DeviceInfoManager& devInfoMgr,
                     FanController& fanCtrl,
                     WiFiManager& wifiMgr)
    : server(server)
    , eepromMgr(eepromMgr)
    , cmdHandler(cmdHandler)
    , i2cComm(i2cComm)
    , devInfoMgr(devInfoMgr)
    , fanCtrl(fanCtrl)
    , wifiMgr(wifiMgr)
{
}

void WebServer::begin() {
    // Initialize SPIFFS
    if(!SPIFFS.begin(true)) {
        Serial.println("[WebServer] SPIFFS Mount Failed");
        return;
    }
    
    setupRoutes();
    server.begin();
    Serial.println("[WebServer] HTTP Server started");
}

void WebServer::setupRoutes() {
    // Serve web interface
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleRoot(request);
    });
    
    // Commands by index
    server.on("/command", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleCommand(request);
    });
    
    // Keystone
    server.on("/keystone", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleKeystone(request);
    });
    
    // Custom command
    server.on("/custom_command", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleCustomCommand(request);
    });
    
    // Test pattern
    server.on("/test_pattern", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleTestPattern(request);
    });
    
    // Set Tx Power
    server.on("/set_tx_power", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleSetTxPower(request);
    });
    
    // Ping
    server.on("/ping", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handlePing(request);
    });
    
    // Get all settings
    server.on("/get_settings", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleGetSettings(request);
    });
    
    // Set settings
    server.on("/set_settings", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleSetSettings(request);
    });
    
    // Set language
    server.on("/set_lang", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleSetLang(request);
    });
    
    // Set picture quality
    server.on("/set_pq", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleSetPQ(request);
    });
    
    // Factory reset
    server.on("/factory_reset", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleFactoryReset(request);
    });
    
    // Save all parameters
    server.on("/save_all", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleSaveAll(request);
    });
    
    // Get device info
    server.on("/get_device_info", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleGetDeviceInfo(request);
    });
    
    // Get temperature
    server.on("/get_temperature", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleGetTemperature(request);
    });
    
    // Get notifications
    server.on("/get_notifications", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleGetNotifications(request);
    });
    
    // Clear EEPROM
    server.on("/clear_eeprom", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleClearEEPROM(request);
    });
    
    // Reboot
    server.on("/reboot", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleReboot(request);
    });
    
    // WiFi scan
    server.on("/wifi_scan", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleWiFiScan(request);
    });
    
    // WiFi list
    server.on("/wifi_list", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleWiFiList(request);
    });
    
    // Set WiFi mode
    server.on("/set_wifi_mode", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleSetWiFiMode(request);
    });
    
    // WiFi connect
    server.on("/wifi_connect", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleWiFiConnect(request);
    });
    
    // WiFi disconnect
    server.on("/wifi_disconnect", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleWiFiDisconnect(request);
    });
    
    // WiFi status
    server.on("/wifi_status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleWiFiStatus(request);
    });
    
    // Set fan mode
    server.on("/set_fan", HTTP_GET, [this](AsyncWebServerRequest* request) {
        this->handleSetFan(request);
    });
}

void WebServer::handleRoot(AsyncWebServerRequest* request) {
    File file = SPIFFS.open("/web_interface.html", "r");
    if (!file) {
        request->send(404, "text/plain", "HTML file not found");
        return;
    }
    
    String htmlContent;
    while(file.available()) {
        htmlContent += file.readString();
    }
    file.close();
    
    request->send(200, "text/html", htmlContent);
}

void WebServer::handleCommand(AsyncWebServerRequest* request) {
    if (!request->hasParam("cmd")) {
        request->send(400, "text/plain", "Missing cmd parameter");
        return;
    }
    
    String cmdStr = request->getParam("cmd")->value();
    int cmdIndex = cmdStr.toInt();
    
    if (cmdIndex < 1 || cmdIndex > 30) {
        request->send(400, "text/plain", "Invalid command index");
        return;
    }
    
    cmdHandler.sendCommandByIndex(cmdIndex);
    request->send(200, "text/plain", "Command executed");
}

void WebServer::handleKeystone(AsyncWebServerRequest* request) {
    if (!request->hasParam("pan") || !request->hasParam("tilt") || !request->hasParam("flip")) {
        request->send(400, "text/plain", "Missing parameters");
        return;
    }
    
    SystemSettings settings = eepromMgr.getSettings();
    settings.pan = request->getParam("pan")->value().toInt();
    settings.pan = constrain(settings.pan, PAN_MIN, PAN_MAX);
    settings.tilt = request->getParam("tilt")->value().toInt();
    settings.tilt = constrain(settings.tilt, TILT_MIN, TILT_MAX);
    settings.flip = request->getParam("flip")->value().toInt();
    if (settings.flip < 0 || settings.flip > 3) settings.flip = 0;
    
    i2cComm.sendKeystoneAndFlip(settings.pan, settings.tilt, settings.flip);
    eepromMgr.saveSettings(settings);
    request->send(200, "text/plain", "Keystone and Flip updated");
}

void WebServer::handleCustomCommand(AsyncWebServerRequest* request) {
    if (!request->hasParam("cmd")) {
        request->send(400, "text/plain", "Missing cmd parameter");
        return;
    }
    
    String customCmd = request->getParam("cmd")->value();
    
    if (customCmd.length() % 2 != 0 || customCmd.length() > 50) {
        request->send(400, "text/plain", "Invalid command format");
        return;
    }
    
    // Hex check
    for (size_t i = 0; i < customCmd.length(); ++i) {
        char c = customCmd[i];
        bool ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        if (!ok) {
            request->send(400, "text/plain", "Invalid hex content");
            return;
        }
    }
    
    cmdHandler.sendCustomCommand(customCmd.c_str());
    request->send(200, "text/plain", "Custom command sent");
}

void WebServer::handleTestPattern(AsyncWebServerRequest* request) {
    if (!request->hasParam("pattern")) {
        request->send(400, "text/plain", "Missing pattern parameter");
        return;
    }
    
    uint8_t pattern = request->getParam("pattern")->value().toInt();
    i2cComm.sendTestPattern(pattern);
    request->send(200, "text/plain", "Test pattern command sent");
}

void WebServer::handleSetTxPower(AsyncWebServerRequest* request) {
    if (!request->hasParam("power")) {
        request->send(400, "text/plain", "Missing power parameter");
        return;
    }
    
    SystemSettings settings = eepromMgr.getSettings();
    settings.txPower = request->getParam("power")->value().toInt();
    wifiMgr.setTxPower(settings.txPower);
    eepromMgr.saveSettings(settings);
    request->send(200, "text/plain", "Transmit Power set to " + String(settings.txPower / 4.0) + " dBm");
}

void WebServer::handlePing(AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "ok");
}

void WebServer::handleGetSettings(AsyncWebServerRequest* request) {
    SystemSettings settings = eepromMgr.getSettings();
    
    String json = "{";
    json += "\"pan\":" + String(settings.pan) + ",";
    json += "\"tilt\":" + String(settings.tilt) + ",";
    json += "\"flip\":" + String(settings.flip) + ",";
    json += "\"txPower\":" + String(settings.txPower) + ",";
    json += "\"lang\":\"" + String((settings.lang == 0) ? "en" : "zh") + "\",";
    json += "\"brightness\":" + String(settings.brightness) + ",";
    json += "\"contrast\":" + String(settings.contrast) + ",";
    json += "\"hueU\":" + String(settings.hueU) + ",";
    json += "\"hueV\":" + String(settings.hueV) + ",";
    json += "\"satU\":" + String(settings.satU) + ",";
    json += "\"satV\":" + String(settings.satV) + ",";
    json += "\"sharpness\":" + String(settings.sharpness) + ",";
    json += "\"fanMode\":" + String(settings.fanMode);
    json += "}";
    
    request->send(200, "application/json", json);
}

void WebServer::handleSetSettings(AsyncWebServerRequest* request) {
    SystemSettings settings = eepromMgr.getSettings();
    bool changed = false;
    
    if (request->hasParam("pan")) {
        settings.pan = request->getParam("pan")->value().toInt();
        settings.pan = constrain(settings.pan, PAN_MIN, PAN_MAX);
        changed = true;
    }
    
    if (request->hasParam("tilt")) {
        settings.tilt = request->getParam("tilt")->value().toInt();
        settings.tilt = constrain(settings.tilt, TILT_MIN, TILT_MAX);
        changed = true;
    }
    
    if (request->hasParam("flip")) {
        settings.flip = request->getParam("flip")->value().toInt();
        if (settings.flip < 0 || settings.flip > 3) settings.flip = 0;
        changed = true;
    }
    
    if (request->hasParam("txPower")) {
        settings.txPower = request->getParam("txPower")->value().toInt();
        wifiMgr.setTxPower(settings.txPower);
        changed = true;
    }
    
    if (request->hasParam("lang")) {
        String l = request->getParam("lang")->value();
        settings.lang = (l == "zh") ? 1 : 0;
        changed = true;
    }
    
    if (changed) {
        i2cComm.sendKeystoneAndFlip(settings.pan, settings.tilt, settings.flip);
        eepromMgr.saveSettings(settings);
    }
    
    request->send(200, "text/plain", "OK");
}

void WebServer::handleSetLang(AsyncWebServerRequest* request) {
    if (!request->hasParam("lang")) {
        request->send(400, "text/plain", "Missing lang");
        return;
    }
    
    SystemSettings settings = eepromMgr.getSettings();
    String l = request->getParam("lang")->value();
    settings.lang = (l == "zh") ? 1 : 0;
    eepromMgr.saveSettings(settings);
    request->send(200, "text/plain", "Lang updated");
}

void WebServer::handleSetPQ(AsyncWebServerRequest* request) {
    SystemSettings settings = eepromMgr.getSettings();
    bool pqChanged = false;
    
    if (request->hasParam("brightness")) {
        settings.brightness = request->getParam("brightness")->value().toInt();
        settings.brightness = constrain(settings.brightness, 0, 255);
        int val = map(settings.brightness, 0, 255, -31, 31);
        i2cComm.sendPictureQualityCommand(0x43, 0x01, (int8_t)val);
        pqChanged = true;
    }
    
    if (request->hasParam("contrast")) {
        settings.contrast = request->getParam("contrast")->value().toInt();
        settings.contrast = constrain(settings.contrast, 0, 255);
        int val = map(settings.contrast, 0, 255, -15, 15);
        i2cComm.sendPictureQualityCommand(0x45, 0x01, (int8_t)val);
        pqChanged = true;
    }
    
    if (request->hasParam("hueU") && request->hasParam("hueV")) {
        settings.hueU = request->getParam("hueU")->value().toInt();
        settings.hueU = constrain(settings.hueU, 0, 255);
        settings.hueV = request->getParam("hueV")->value().toInt();
        settings.hueV = constrain(settings.hueV, 0, 255);
        int u = map(settings.hueU, 0, 255, -15, 15);
        int v = map(settings.hueV, 0, 255, -15, 15);
        Wire.beginTransmission(I2C_ADDRESS);
        Wire.write(0x47); // Set Hue
        Wire.write(0x02); // OP0=2
        Wire.write((int8_t)u); // OP1: U
        Wire.write((int8_t)v); // OP2: V
        uint8_t error = Wire.endTransmission();
        Serial.printf("[PQ] Set Hue U/V: %d/%d, I2C error=%d\n", u, v, error);
        pqChanged = true;
    }
    
    if (request->hasParam("satU") && request->hasParam("satV")) {
        settings.satU = request->getParam("satU")->value().toInt();
        settings.satU = constrain(settings.satU, 0, 255);
        settings.satV = request->getParam("satV")->value().toInt();
        settings.satV = constrain(settings.satV, 0, 255);
        int u = map(settings.satU, 0, 255, -15, 15);
        int v = map(settings.satV, 0, 255, -15, 15);
        Wire.beginTransmission(I2C_ADDRESS);
        Wire.write(0x49); // Set Saturation
        Wire.write(0x02); // OP0=2
        Wire.write((int8_t)u); // OP1: U
        Wire.write((int8_t)v); // OP2: V
        Wire.endTransmission();
        pqChanged = true;
    }
    
    if (request->hasParam("sharpness")) {
        settings.sharpness = request->getParam("sharpness")->value().toInt();
        settings.sharpness = constrain(settings.sharpness, 0, 255);
        int val = map(settings.sharpness, 0, 255, 0, 8);
        i2cComm.sendPictureQualityCommand(0x4F, 0x01, (uint8_t)val);
        pqChanged = true;
    }
    
    if (pqChanged) {
        eepromMgr.saveSettings(settings);
    }
    
    request->send(200, "text/plain", "PQ updated");
}

void WebServer::handleFactoryReset(AsyncWebServerRequest* request) {
    i2cComm.sendFactoryResetCommand();
    request->send(200, "text/plain", "Factory reset command sent.");
}

void WebServer::handleSaveAll(AsyncWebServerRequest* request) {
    i2cComm.sendSaveAllCommand();
    request->send(200, "text/plain", "Save all command sent.");
}

void WebServer::handleGetDeviceInfo(AsyncWebServerRequest* request) {
    // Request all device information
    devInfoMgr.requestAll();
    
    String json = "{";
    json += "\"temperature\":{\"current\":" + String(devInfoMgr.getTemperature()) +
            ",\"lower\":" + String(devInfoMgr.getMuteThreshold()) +
            ",\"upper\":" + String(devInfoMgr.getStopThreshold()) + "},";
    json += "\"runtime\":" + String(devInfoMgr.getRuntime()) + ",";
    json += "\"version\":{\"firmware\":\"" + devInfoMgr.getFirmwareVersion() +
            "\",\"parameter\":\"" + devInfoMgr.getParameterVersion() +
            "\",\"data\":\"" + devInfoMgr.getDataVersion() + "\"},";
    json += "\"lot_number\":\"" + devInfoMgr.getLotNumber() + "\",";
    json += "\"serial_number\":\"" + devInfoMgr.getSerialNumber() + "\"";
    json += "}";
    
    Serial.println("[WebServer] Sending device info: " + json);
    request->send(200, "application/json", json);
}

void WebServer::handleGetTemperature(AsyncWebServerRequest* request) {
    devInfoMgr.requestTemperature();
    
    String json = "{\"temperature\":" + String(devInfoMgr.getTemperature()) +
                  ",\"mute_threshold\":" + String(devInfoMgr.getMuteThreshold()) +
                  ",\"stop_threshold\":" + String(devInfoMgr.getStopThreshold()) + "}";
    request->send(200, "application/json", json);
}

void WebServer::handleGetNotifications(AsyncWebServerRequest* request) {
    // Return system status information
    String json = "{\"notifications\":[]}";
    request->send(200, "application/json", json);
}

void WebServer::handleClearEEPROM(AsyncWebServerRequest* request) {
    eepromMgr.clearAll();
    request->send(200, "text/plain", "EEPROM cleared. Please restart the device.");
}

void WebServer::handleReboot(AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "rebooting");
    delay(100);
    ESP.restart();
}

void WebServer::handleWiFiScan(AsyncWebServerRequest* request) {
    wifiMgr.requestScan();
    
    String json = "{";
    json += "\"scanning\":" + String(wifiMgr.isScanning() ? "true" : "false") + ",";
    json += "\"networks\":" + wifiMgr.getScanResults();
    json += "}";
    
    request->send(200, "application/json", json);
}

void WebServer::handleWiFiList(AsyncWebServerRequest* request) {
    request->send(200, "application/json", wifiMgr.getScanResults());
}

void WebServer::handleSetWiFiMode(AsyncWebServerRequest* request) {
    if (!request->hasParam("mode")) {
        request->send(400, "text/plain", "Missing mode");
        return;
    }
    
    String mode = request->getParam("mode")->value();
    
    if (mode == "sta") {
        SystemSettings settings = eepromMgr.getSettings();
        settings.wifiConfigured = true;
        eepromMgr.saveSettings(settings);
        request->send(200, "text/plain", "STA enabled, rebooting...");
        delay(100);
        ESP.restart();
    } else {
        SystemSettings settings = eepromMgr.getSettings();
        settings.wifiConfigured = false;
        eepromMgr.saveSettings(settings);
        wifiMgr.startAPMode();
        request->send(200, "text/plain", "AP only mode enabled.");
    }
}

void WebServer::handleWiFiConnect(AsyncWebServerRequest* request) {
    if (!request->hasParam("ssid")) {
        request->send(400, "text/plain", "Missing ssid");
        return;
    }
    
    String ssid = request->getParam("ssid")->value();
    String pwd = request->hasParam("pwd") ? request->getParam("pwd")->value() : "";
    
    if (ssid.length() == 0 || ssid.length() > 32) {
        request->send(400, "text/plain", "Invalid SSID");
        return;
    }
    
    if (pwd.length() > 64) {
        request->send(400, "text/plain", "Password too long");
        return;
    }
    
    SystemSettings settings = eepromMgr.getSettings();
    settings.ssid = ssid;
    settings.pwd = pwd;
    eepromMgr.saveSettings(settings);
    
    String resp = "Credentials saved for: " + ssid + ". Switch to STA mode to connect.";
    request->send(200, "text/plain", resp);
}

void WebServer::handleWiFiDisconnect(AsyncWebServerRequest* request) {
    wifiMgr.disconnect();
    
    SystemSettings settings = eepromMgr.getSettings();
    settings.wifiConfigured = false;
    settings.ssid = "";
    settings.pwd = "";
    eepromMgr.saveSettings(settings);
    
    wifiMgr.startAPMode();
    request->send(200, "text/plain", "Disconnected and returned to AP mode");
}

void WebServer::handleWiFiStatus(AsyncWebServerRequest* request) {
    SystemSettings settings = eepromMgr.getSettings();
    request->send(200, "application/json", wifiMgr.getStatusJSON(settings.lang));
}

void WebServer::handleSetFan(AsyncWebServerRequest* request) {
    if (!request->hasParam("mode")) {
        request->send(400, "text/plain", "missing mode");
        return;
    }
    
    uint8_t mode = request->getParam("mode")->value().toInt();
    
    SystemSettings settings = eepromMgr.getSettings();
    settings.fanMode = mode;
    eepromMgr.saveSettings(settings);
    
    fanCtrl.setMode(mode);
    request->send(200, "text/plain", "OK");
}
