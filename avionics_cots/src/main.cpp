/**
 * @file main.cpp
 * @author Eliot Abramo
*/
#include <Arduino.h>
// #include "Mass_thread.hpp"
#include "packet_definition.hpp"
#include "Cosco.hpp"
#include "Servo.hpp"

//static HX711Thread mass_thread;
// HX711Thread* mass_thread = new HX711Thread();

// MassConfigRequestPacket latest_mass_config_request = { .id = 3 + 0x100, .req_config = true };
// MassConfigResponsePacket latest_mass_config_response = { .id = 3, .offset = 1.0, .scale = 2.0, .offset_set = true, .scale_set = false };

Cosco cosco;

Servo_Driver* servo_cam = new Servo_Driver();


void setup() {
  // mass_thread->init();
  servo_cam->init(13, 0);
  Serial.begin(115200);
  
}
#if 1
void loop() {
  cosco.receive(servo_cam);

  // // below is a test to send dust data every 2 seconds
  // static uint32_t last_send = 0;
  // if (millis() - last_send >= 2000) {
  //   DustData dust_packet = {
  //     .pm1_0_std = 1,
  //     .pm2_5_std = 0,
  //     .pm10__std = 2,
  //     .pm1_0_atm = 0,
  //     .pm2_5_atm = 3,
  //     .pm10__atm = 0,
  //     .num_particles_0_3 = 4,
  //     .num_particles_0_5 = 0,
  //     .num_particles_1_0 = 5,
  //     .num_particles_2_5 = 0,
  //     .num_particles_5_0 = 6,
  //     .num_particles_10_ = 0
  //   };
  //   cosco.sendDustDataPacket(&dust_packet);
  //   last_send = millis();
  // }

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