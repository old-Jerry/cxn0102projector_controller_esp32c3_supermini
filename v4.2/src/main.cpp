// modify by Lyu on 2025/11/18
// Refactored to modular architecture - 2026

#include <Arduino.h>
#include <Wire.h>
#include <SPIFFS.h>
#include "esp_system.h"
#include "esp_log.h"

// Module headers
#include "config.h"
#include "eeprom_manager.h"
#include "wifi_manager.h"
#include "i2c_communicator.h"
#include "command_handler.h"
#include "fan_controller.h"
#include "device_info.h"
#include "web_server.h"

// Global module instances
AsyncWebServer server(80);
EEPROMManager eepromManager;
WiFiManager wifiManager(server);
I2CCommunicator i2cComm;
CommandHandler commandHandler;
FanController fanController;
DeviceInfoManager deviceInfoManager;
WebServer webServer;

// Notify callback for I2C
void notifyCallback(uint8_t cmd, uint8_t size, uint8_t result, const uint8_t* data, uint8_t length);

// Button interrupt
void IRAM_ATTR buttonISR() {
    // Button press will be handled in loop
}

void setup() {
    Serial.begin(115200);
    delay(5000);
    
    ESP_LOGI("MAIN", "CXN0102 Controller V4.2 - Modular Architecture");
    Serial.println("Initializing system...");
    
    // Initialize EEPROM and load settings
    eepromManager.begin();
    SystemSettings settings = eepromManager.getSettings();
    
    // Initialize Fan PWM
    fanController.begin();
    fanController.setMode(settings.fanMode);
    Serial.println("[Main] Fan controller initialized");
    
    // Initialize I2C with notify callback
    i2cComm.begin(notifyCallback);
    Serial.println("[Main] I2C communicator initialized");
    
    // Initialize Device Info Manager
    deviceInfoManager.setI2CCommunicator(&i2cComm);
    Serial.println("[Main] Device info manager initialized");
    
    // Initialize WiFi
    wifiManager.begin();
    if (settings.wifiConfigured) {
        Serial.println("[Main] WiFi configured, attempting STA mode");
        wifiManager.startSTAMode(settings.ssid, settings.pwd);
    } else {
        Serial.println("[Main] WiFi not configured, starting AP mode");
        wifiManager.startAPMode();
    }
    
    // Set Tx power
    wifiManager.setTxPower(settings.txPower);
    
    // Apply geometry settings via I2C
    i2cComm.sendKeystoneAndFlip(settings.pan, settings.tilt, settings.flip);
    
    // Apply picture quality settings
    i2cComm.sendPictureQuality(settings);
    
    // Initialize Web Server
    webServer.begin();
    Serial.println("[Main] Web server initialized");
    
    // Start HTTP server
    server.begin();
    Serial.println("[Main] HTTP server started");
    
    // Initialize button pin
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    
    // Wait for system to stabilize
    delay(2000);
    
    // Request all device info
    deviceInfoManager.requestAll();
    Serial.println("[Main] Device info requested");
    
    // Auto send Start Input command
    commandHandler.sendCommandByIndex(0);
    Serial.println("[Main] Start Input command sent");
    
    Serial.println("[Main] ===== System initialized successfully =====");
}

void loop() {
    static unsigned long lastInfoUpdate = 0;
    static unsigned long lastFanCheck = 0;
    static unsigned long lastWiFiCheck = 0;
    
    // Process I2C notifications
    i2cComm.processNotify();
    
    // Process WiFi scan
    wifiManager.processScan();
    
    // Check WiFi reconnection and fallback
    wifiManager.checkReconnectFallback();
    
    // Update device info every 60 seconds
    if (millis() - lastInfoUpdate > 60000) {
        deviceInfoManager.requestTemperature();
        lastInfoUpdate = millis();
    }
    
    // Check WiFi status every 10 seconds
    if (millis() - lastWiFiCheck > 10000) {
        if (wifiManager.isWifiConfigured() && !wifiManager.isWaitingForWiFi()) {
            // STA mode but disconnected, attempt reconnect
            WiFi.reconnect();
        }
        lastWiFiCheck = millis();
    }
    
    // Adjust fan speed every 60 seconds
    if (millis() - lastFanCheck > 60000) {
        uint8_t mode = fanController.getMode();
        if (mode == 0 || mode == 1 || mode == 2 || mode == 3) {
            int temp = deviceInfoManager.getTemperature();
            if (temp >= 0) {
                fanController.adjustSpeed(temp);
            }
        }
        lastFanCheck = millis();
    }
    
    // Handle button press (Stop + Shutdown)
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50);
        if (digitalRead(BUTTON_PIN) == LOW) {
            commandHandler.sendCommandByIndex(1); // Stop Input
            delay(100);
            commandHandler.sendCommandByIndex(3); // Shutdown
            Serial.println("[Main] Shutdown command sent via button");
            while (digitalRead(BUTTON_PIN) == LOW) { 
                delay(10); 
            }
        }
    }
    
    delay(10);
}

// Notify callback implementation
void notifyCallback(uint8_t cmd, uint8_t size, uint8_t result, const uint8_t* data, uint8_t length) {
    switch (cmd) {
        case 0x00: // Boot Completed
            Serial.println("[Notify] Boot Completed");
            if (result != 0x00) {
                Serial.printf("[Notify] Boot error: 0x%02X\n", result);
            }
            break;
        
        case 0x10: // Emergency
            Serial.printf("[Notify] Emergency: 0x%02X\n", result);
            break;
        
        case 0x11: // Temperature Emergency and Recovery
            if (result == 0x80 || result == 0x81) {
                Serial.printf("[Notify] Temperature Emergency: 0x%02X\n", result);
            } else {
                Serial.printf("[Notify] Temperature Recovery: 0x%02X\n", result);
            }
            break;
        
        case 0x12: // Command Error
            Serial.printf("[Notify] Command Error: 0x%02X\n", result);
            break;
        
        case 0xA0: // Temperature info
            if (length >= 6 && result == 0x00) {
                deviceInfoManager.updateTemperature(data[3], data[4], data[5]);
            }
            break;
        
        default:
            Serial.printf("[Notify] Unknown command: 0x%02X\n", cmd);
            break;
    }
}
