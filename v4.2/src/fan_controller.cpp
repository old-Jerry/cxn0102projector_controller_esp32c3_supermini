#include "fan_controller.h"
#include "config.h"

FanController::FanController() : fanMode(DEFAULT_FAN_MODE), fanPwmValue(0) {
}

void FanController::begin() {
    ledcSetup(FAN_PWM_CHANNEL, FAN_PWM_FREQ, FAN_PWM_RES);
    ledcAttachPin(FAN_PWM_PIN, FAN_PWM_CHANNEL);
    
    // 应用初始模式
    setMode(fanMode);
    
    Serial.println("[Fan] PWM initialized on pin 12");
}

void FanController::setMode(uint8_t mode) {
    fanMode = mode;
    
    if (fanMode == 4) {
        // Full模式：最大PWM
        fanPwmValue = 255;
        ledcWrite(FAN_PWM_CHANNEL, fanPwmValue);
        Serial.printf("[Fan] Full mode, PWM=%u\n", fanPwmValue);
    } else {
        // 其他模式：基于温度调整
        adjustSpeed(-1); // -1 表示暂时没有温度数据，使用默认值
        Serial.printf("[Fan] Mode=%d enabled, PWM will adjust based on temperature\n", fanMode);
    }
}

uint8_t FanController::getMode() const {
    return fanMode;
}

uint8_t FanController::getPWM() const {
    return fanPwmValue;
}

FanController::FanCurve FanController::getCurveForMode(uint8_t mode) {
    switch (mode) {
        case 0: // Silent: 静音模式，低温就慢慢启动，平缓曲线
            return {25, 50, 30, 120, 0.5};
        case 1: // Normal: 正常模式
            return {25, 70, 60, 220, 1.2};
        case 2: // Aggressive: 激进模式，低温启动，陡峭曲线
            return {25, 65, 80, 255, 1.8};
        case 3: // Auto: 自动模式（平衡）
        default:
            return {25, 70, 50, 200, 1.0};
    }
}

void FanController::adjustSpeed(int temperature) {
    if (fanMode == 4) {
        // Full模式不调整
        return;
    }
    
    // 如果没有有效温度，使用默认值
    if (temperature < 0) {
        temperature = 40; // 默认40°C
    }
    
    FanCurve curve = getCurveForMode(fanMode);
    
    // 限制温度在有效范围内
    temperature = constrain(temperature, curve.temp_min, curve.temp_max);
    
    // 计算归一化位置 (0.0 ~ 1.0)
    float normalized = (float)(temperature - curve.temp_min) / (float)(curve.temp_max - curve.temp_min);
    
    // 应用曲线函数
    float curved;
    if (curve.curve_factor == 1.0) {
        curved = normalized; // 线性
    } else if (curve.curve_factor > 1.0) {
        curved = pow(normalized, curve.curve_factor); // 陡峭曲线
    } else {
        curved = 1.0 - pow(1.0 - normalized, 1.0 / curve.curve_factor); // 平缓曲线
    }
    
    // 计算PWM值
    fanPwmValue = (uint8_t)(curve.pwm_min + curved * (curve.pwm_max - curve.pwm_min));
    
    // 确保PWM在有效范围内
    fanPwmValue = constrain(fanPwmValue, curve.pwm_min, curve.pwm_max);
    
    ledcWrite(FAN_PWM_CHANNEL, fanPwmValue);
    
    Serial.printf("[Fan] Mode=%d Temp=%d°C PWM=%u (Curve: %d-%d°C -> %d-%d PWM, factor=%.1f)\n",
                  fanMode, temperature, fanPwmValue,
                  curve.temp_min, curve.temp_max,
                  curve.pwm_min, curve.pwm_max,
                  curve.curve_factor);
}

void FanController::setPWM(uint8_t pwm) {
    fanPwmValue = pwm;
    ledcWrite(FAN_PWM_CHANNEL, fanPwmValue);
    Serial.printf("[Fan] PWM set to %u\n", fanPwmValue);
}
