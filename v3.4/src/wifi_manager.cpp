#include "wifi_manager.h"
#include "config.h"
#include "esp_log.h"

const char* AP_SSID = "CXN0102_Web_Controller";
const char* AP_PASSWORD = "12345678"; // 必须 ≥ 8 字符

WiFiManager::WiFiManager() : _initialized(false) {
}

void WiFiManager::init() {
    if (!_initialized) {
        Serial.println("[WiFiManager] Starting WiFi SoftAP...");
        WiFi.softAP(AP_SSID, AP_PASSWORD);
        _initialized = true;
        Serial.println("[WiFiManager] WiFi SoftAP started.");
        Serial.print("[WiFiManager] AP IP address: ");
        Serial.println(WiFi.softAPIP());
    }
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
    Serial.printf("[WiFiManager] Transmit power set to %.1f dBm (code: %d)\n", 
                 getTxPowerdBm(powerLevel), powerLevel);
}

float WiFiManager::getTxPowerdBm(int powerLevel) {
    return powerLevel / 4.0;
}
