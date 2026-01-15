#ifndef FAN_CONTROLLER_H
#define FAN_CONTROLLER_H

#include <Arduino.h>

class FanController {
public:
    FanController();
    
    // 初始化风扇PWM
    void begin();
    
    // 设置风扇模式
    // 0=Silent, 1=Normal, 2=Aggressive, 3=Auto, 4=Full
    void setMode(uint8_t mode);
    
    // 获取当前模式
    uint8_t getMode() const;
    
    // 获取当前PWM值
    uint8_t getPWM() const;
    
    // 根据温度调整风扇速度
    void adjustSpeed(int temperature);
    
    // 直接设置PWM值
    void setPWM(uint8_t pwm);
    
private:
    uint8_t fanMode;
    uint8_t fanPwmValue;
    
    // 风扇曲线结构
    struct FanCurve {
        int temp_min;
        int temp_max;
        uint8_t pwm_min;
        uint8_t pwm_max;
        float curve_factor; // 曲线系数
    };
    
    FanCurve getCurveForMode(uint8_t mode);
};

#endif // FAN_CONTROLLER_H
