/**
 * @file Servo.hpp
 * @author Eliot Abramo
*/
#ifndef DUST_SENSOR_HPP
#define DUST_SENSOR_HPP


#include <iostream>
#include "driver/ledc.h"

class Dust
{
public:
     Dust();
    ~Dust();

    void init();
    void loop();
    
    void calibrate();

private:
    //
};

#endif /** DUST_SENSOR_HPP */