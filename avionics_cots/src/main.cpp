/**
 * @file main.cpp
 * @author Eliot Abramo
*/
#include <Arduino.h>
#include "Mass_thread.hpp"
#include "packet_definition.hpp"

//static HX711Thread mass_thread;
// HX711Thread* mass_thread = new HX711Thread();

MassConfigRequestPacket latest_mass_config_request = { .id = 3 + 0x100, .req_config = true };
MassConfigResponsePacket latest_mass_config_response = { .id = 3, .offset = 1.0, .scale = 2.0, .offset_set = true, .scale_set = false };

Cosco cosco;

void setup() {
  // mass_thread->init();
  Serial.begin(115200);
  
}
#if 1
void loop() {
  cosco.receive();  // Check and process any incoming packets
  // static unsigned long last_send = 0;
  //   if (millis() - last_send >= 2000) {
  //     cosco.sendMassConfigRequestPacket(&latest_mass_config_request);
  //     cosco.sendMassConfigResponsePacket(&latest_mass_config_response);
  //     last_send = millis();
  //   }
}
#else
void loop() {
  Serial.println("Hello World");
  delay(1000);
}
#endif