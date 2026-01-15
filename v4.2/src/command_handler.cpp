#include "command_handler.h"
#include "config.h"
#include <Wire.h>

// 预定义命令表
const char* CommandHandler::commands[] = {
    "0100", // 1: Start Input
    "0200", // 2: Stop Input
    "0b0101", // 3: Reboot
    "0b0100", // 4: Shutdown
    "3200", // 5: Enter Optical Axis Adjustment
    "3300", // 6: Optical Axis +
    "3400", // 7: Optical Axis -
    "350100", // 8: Exit Optical Axis (No Save)
    "350101", // 9: Exit Optical Axis (Save)
    "3600", // 10: Enter Bi-Phase Adjustment
    "3700", // 11: Bi-Phase +
    "3800", // 12: Bi-Phase -
    "390100", // 13: Exit Bi-Phase (No Save)
    "390101", // 14: Exit Bi-Phase (Save)
    "4A", // 15: Flip Mode
    "5001", // 16: Test Image ON
    "5000", // 17: Test Image OFF
    "6000", // 18: Mute
    "6001", // 19: Unmute
    "7000", // 20: Keystone Vertical -
    "7001", // 21: Keystone Vertical +
    "7002", // 22: Keystone Horizontal -
    "7003", // 23: Keystone Horizontal +
    "8000", // 24: Color Temperature -
    "8001", // 25: Color Temperature +
    "4300", // 26: Set Brightness (默认值)
    "4500", // 27: Set Contrast
    "4700", // 28: Set Hue
    "4900", // 29: Set Saturation
    "4F00", // 30: Set Sharpness
};

const int CommandHandler::commandCount = sizeof(commands) / sizeof(commands[0]);

CommandHandler::CommandHandler() {
}

void CommandHandler::sendCommandByIndex(int index) {
    if (index < 1 || index > commandCount) {
        Serial.printf("[CMD] Invalid command index: %d\n", index);
        return;
    }
    sendCustomCommand(commands[index - 1]);
}

void CommandHandler::sendCustomCommand(const char* cmd) {
    Wire.beginTransmission(I2C_ADDRESS);
    for (int i = 0; i < (int)strlen(cmd); i += 2) {
        char byteStr[3] = {cmd[i], cmd[i + 1], '\0'};
        uint8_t byteVal = (uint8_t)strtol(byteStr, NULL, 16);
        Wire.write(byteVal);
    }
    uint8_t error = Wire.endTransmission();
    if (error) {
        Serial.printf("[CMD] I2C error: %d\n", error);
    } else {
        Serial.println("[CMD] Command sent successfully.");
    }
}

int CommandHandler::getCommandCount() const {
    return commandCount;
}

const char* CommandHandler::getCommand(int index) const {
    if (index < 0 || index >= commandCount) {
        return nullptr;
    }
    return commands[index];
}
