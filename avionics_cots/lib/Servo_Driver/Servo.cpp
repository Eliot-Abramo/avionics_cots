/**
 * @file Servo.cpp
 * @author Eliot Abramo
*/
#include "Servo.hpp"

Servo::Servo(ledc_channel_config_t* ledc, uint8_t channel, uint16_t PWM_Pin)
            :_ledc(ledc), _PWM_Pin(PWM_Pin) 
{
    switch (channel)
    {
    case 1:
        break;
    
    default:
        break;
    }
}