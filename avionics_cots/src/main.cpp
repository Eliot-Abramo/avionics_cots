/**
 * @file main.cpp
 * @author Eliot Abramo
*/
#include <Arduino.h>
#include "ADS1234.hpp"
#include "ADS1234_Mass_Thread.hpp"

static ADS1234Thread mass_thread;

void setup() {
  mass_thread.init();
}

void loop() {
  mass_thread.loop();
}
