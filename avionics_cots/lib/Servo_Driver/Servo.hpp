/**
 * @file Servo.hpp
 * @author Eliot Abramo
*/
#ifndef SERVO_HPP
#define SERVO_HPP


#include <iostream>
#include "driver/ledc.h"
#define DEFAULT_PWM_PERIOD 20000-1

class Servo
{
public:
    Servo(ledc_channel_config_t* ledc, uint8_t channel, uint16_t PWM_Pin);
    ~Servo();

    void set_servo(uint32_t duty);

private:
    uint8_t _channel;
    ledc_channel_config_t* _ledc;

    uint16_t _PWM_Pin;
    uint32_t _period = DEFAULT_PWM_PERIOD;
};

#endif