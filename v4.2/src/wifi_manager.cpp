#include "wifi_manager.h"
#include "config.h"

WiFiManager::WiFiManager(AsyncWebServer& server) 
    : server(server), wifiManager(&server, nullptr), 
      wifiConfigured(false), scanningWiFi(false), scanRequested(false),
      scanStartTime(0), wifiScanResults("[]"), waitingForWiFi(false),
      connectStartTime(0) {
}

void WiFiManager::begin() {
    Serial.println("[WiFi] Initializing WiFi Manager");
}

void WiFiManager::startAPMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(
        IPAddress(192, 168, 4, 1),
        IPAddress(192, 168, 4, 1),
        IPAddress(255, 255, 255, 0)
    );
    bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD, 1, 0, 8);
    if (apStarted) {
        Serial.println("[WiFi] AP started successfully");
        Serial.print("[WiFi] SSID: "); Serial.println(AP_SSID);
        Serial.print("[WiFi] IP address: "); Serial.println(WiFi.softAPIP());
        Serial.print("[WiFi] MAC address: "); Serial.println(WiFi.softAPmacAddress());
    } else {
        Serial.println("[WiFi] Failed to start AP!");
    }
    
    // 启动时扫描并缓存
    startScan();
}

void WiFiManager::startSTAMode(const String& ssid, const String& pwd) {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    
    // 使用提供的SSID和PWD尝试连接
    if (ssid.length() > 0) {
        WiFi.begin(ssid.c_str(), pwd.c_str());
        connectStartTime = millis();
        waitingForWiFi = true;
        Serial.printf("[WiFi] Trying to connect to SSID: %s\n", ssid.c_str());
    } else {
        Serial.println("[WiFi] No credentials provided, fallback to AP.");
        wifiConfigured = false;
        startAPMode();
    }
}

void WiFiManager::setWifiConfigured(bool configured) {
    wifiConfigured = configured;
}

bool WiFiManager::isWifiConfigured() const {
    return wifiConfigured;
}

void WiFiManager::setTxPower(int powerLevel) {
    wifi_power_t txPower;
    switch (powerLevel) {
        case 78: txPower = WIFI_POWER_19_5dBm; break;
        case 76: txPower = WIFI_POWER_19dBm; break;
        case 74: txPower = WIFI_POWER_18_5dBm; break;
        case 68: txPower = WIFI_POWER_17dBm; break;
        case 60: txPower = WIFI_POWER_15dBm; break;
        case 52: txPower = WIFI_POWER_13dBm; break;
        case 44: txPower = WIFI_POWER_11dBm; break;
        case 34: txPower = WIFI_POWER_8_5dBm; break;
        case 28: txPower = WIFI_POWER_7dBm; break;
        case 20: txPower = WIFI_POWER_5dBm; break;
        case 8:  txPower = WIFI_POWER_2dBm; break;
        case -4: txPower = WIFI_POWER_MINUS_1dBm; break;
        default: txPower = WIFI_POWER_8_5dBm; break;
    }
    WiFi.setTxPower(txPower);
}

void WiFiManager::startScan() {
    if (scanningWiFi) return;
    
    Serial.println("[WiFi] Starting WiFi scan...");
    scanningWiFi = true;
    scanStartTime = millis();
    scanRequested = false;
    
    // 切换到AP+STA模式进行扫描
    WiFi.mode(WIFI_AP_STA);
    WiFi.scanNetworks(true); // 异步扫描
}

void WiFiManager::processScan() {
    if (!scanningWiFi) return;
    
    int scanResult = WiFi.scanComplete();
    if (scanResult == WIFI_SCAN_RUNNING) {
        // 检查超时
        if (millis() - scanStartTime > SCAN_TIMEOUT) {
            WiFi.scanDelete();
            scanningWiFi = false;
            Serial.println("[WiFi] Scan timeout");
            wifiScanResults = "[]";
            
            // 扫描完成后恢复AP模式
            WiFi.mode(WIFI_AP);
            Serial.println("[WiFi] Restored AP mode after scan");
        }
        return;
    }
    
    scanningWiFi = false;
    
    if (scanResult == WIFI_SCAN_FAILED) {
        Serial.println("[WiFi] Scan failed");
        wifiScanResults = "[]";
        // 恢复AP模式
        WiFi.mode(WIFI_AP);
        Serial.println("[WiFi] Restored AP mode after failed scan");
        return;
    }
    
    // 构建扫描结果JSON
    String json = "[";
    for (int i = 0; i < scanResult; ++i) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"encryption\":" + String(WiFi.encryptionType(i)) + ",";
        json += "\"channel\":" + String(WiFi.channel(i));
        json += "}";
    }
    json += "]";
    
    wifiScanResults = json;
    Serial.printf("[WiFi] Scan completed: %d networks found\n", scanResult);
    
    // 清理扫描结果
    WiFi.scanDelete();
    
    // 扫描完成后恢复AP模式
    WiFi.mode(WIFI_AP);
    Serial.println("[WiFi] Restored AP mode after scan completion");
}

void WiFiManager::enableMDNS() {
    if (MDNS.begin("cxn0102")) {
        MDNS.addService("http", "tcp", 80);
        Serial.println("[mDNS] Started: cxn0102.local");
    } else {
        Serial.println("[mDNS] Failed to start");
    }
}

void WiFiManager::checkReconnectFallback() {
    if (waitingForWiFi) {
        wl_status_t status = WiFi.status();
        
        if (status == WL_CONNECTED) {
            waitingForWiFi = false;
            Serial.print("[WiFi] Connected. IP: ");
            Serial.println(WiFi.localIP());
            
            WiFi.softAPdisconnect(true);
            enableMDNS();
        }
        // 只有找不到 SSID 才回到 AP
        else if (status == WL_NO_SSID_AVAIL) {
            waitingForWiFi = false;
            wifiConfigured = false;
            Serial.println("[WiFi] Saved SSID not found, fallback to AP.");
            startAPMode();
        }
        // 连接失败：继续尝试，不切 AP
        else if (status == WL_CONNECT_FAILED) {
            Serial.println("[WiFi] Connection failed, retrying...");
            WiFi.disconnect();
            // 需要重新获取SSID和密码，这里简化处理
            WiFi.reconnect();
            connectStartTime = millis();
        }
        // 超时 1 分钟：继续保持 STA，不切 AP，但可重新尝试
        else if (millis() - connectStartTime > 60000) {
            Serial.println("[WiFi] Connect timeout, retrying STA...");
            WiFi.disconnect();
            WiFi.reconnect();
            connectStartTime = millis();
        }
    }
}

bool WiFiManager::isScanning() const {
    return scanningWiFi;
}

bool WiFiManager::isWaitingForWiFi() const {
    return waitingForWiFi;
}

String WiFiManager::getScanResults() const {
    return wifiScanResults;
}

void WiFiManager::requestScan() {
    scanRequested = true;
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true);
    waitingForWiFi = false;
    
    // 回到AP模式
    startAPMode();
}

String WiFiManager::getWifiStatusJSON(uint8_t lang) const {
    String json = "{";
    json += "\"mode\":\"" + String(wifiConfigured ? "sta" : "ap") + "\",";
    json += "\"connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
    json += "\"ip\":\"" + (WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString()) + "\",";
    json += "\"ssid\":\"" + (WiFi.status() == WL_CONNECTED ? WiFi.SSID() : "CXN0102_Web_Controller") + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"lang\":\"" + String(lang == 1 ? "zh" : "en") + "\"";
    json += "}";
    return json;
}
