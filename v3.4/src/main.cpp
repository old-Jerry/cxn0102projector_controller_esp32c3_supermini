/**
 * CXN0102 Projector Controller - Main Entry
 * Version: v3.4
 * Author: vx:samzhangxian
 * 
 * Refactored into modular structure:
 * - config.h: Configuration and pin definitions
 * - eeprom_handler.h/cpp: EEPROM storage management
 * - cxn0102_i2c.h/cpp: CXN0102 I2C communication
 * - wifi_manager.h/cpp: WiFi SoftAP management
 * - web_server.h/cpp: Web server and HTTP routes
 */

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "eeprom_handler.h"
#include "cxn0102_i2c.h"
#include "wifi_manager.h"
#include "web_server.h"

// Global instances
EEPROMHandler eeprom;
CXN0102I2C cxn0102;
WiFiManager wifiMgr;
WebServerHandler webServer(cxn0102, eeprom);

// Settings
Settings currentSettings;

// Button state
const unsigned long DEBOUNCE_DELAY = 50;

void setup() {
  Serial.begin(9600);
  delay(5000); // Keep original delay
  Serial.println("[MAIN] CXN0102 Controller v3.4 starting...");

  // Initialize button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize EEPROM and load settings
  eeprom.init();
  eeprom.loadSettings(currentSettings);
  Serial.println("[MAIN] Settings loaded from EEPROM");

  // Initialize WiFi SoftAP
  wifiMgr.init();
  wifiMgr.setTxPower(currentSettings.txPower);

  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(COM_REQ_PIN, INPUT);
  cxn0102.init();

  // Apply saved settings to device
  cxn0102.sendKeystoneAndFlip(currentSettings.pan, currentSettings.tilt, currentSettings.flip);
  
  // Initialize web server
  webServer.init();

  Serial.println("[MAIN] System ready. Access http://" + String(AP_SSID));

  // Auto send Start Input after boot (as original behavior)
  cxn0102.sendCommand(CXNCommand::CMD_START_INPUT);
  Serial.println("[MAIN] Start Input command sent automatically.");
}

void loop() {
  // Button press handling (active low)
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(DEBOUNCE_DELAY);
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("[MAIN] Button pressed - sending Stop + Shutdown");
      cxn0102.sendCommand(CXNCommand::CMD_STOP_INPUT);
      delay(100);
      cxn0102.sendCommand(CXNCommand::CMD_SHUTDOWN);
      
      // Wait for button release
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }
      Serial.println("[MAIN] Shutdown sequence completed");
    }
  }
  
  delay(10);
}
