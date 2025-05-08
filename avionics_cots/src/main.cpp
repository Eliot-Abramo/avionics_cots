/**
 * @file main.cpp
 * @author Eliot Abramo
*/
#include <Arduino.h>
#include "HX711.h"
#include <Seeed_HM330X.h>
#include "soc/rtc.h"
#include "packet_definition.hpp"
#include "Cosco.hpp"
#include "Servo.hpp"
#include "Dust_Driver.hpp"

Cosco cosco;

Servo_Driver* servo_cam = new Servo_Driver();
Servo_Driver* servo_drill = new Servo_Driver();
HX711 mass_drill;
HX711 mass_hd;
Dust* dust = new Dust();

/**
 * servo id 1 = cam front
 * servo id 2 = drill clapet 
 * mass id 3 = mass drill
 * mass id 4 = mass rover
 */

void setup() {
  rtc_cpu_freq_config_t config;
  rtc_clk_cpu_freq_get_config(&config);
  rtc_clk_cpu_freq_to_config(RTC_CPU_FREQ_80M, &config);
  rtc_clk_cpu_freq_set_config_fast(&config);
  
  servo_cam->init(13, 0);
  servo_drill->init(12, 1);

  mass_drill.begin(16, 4);
  mass_drill.set_scale(-395.606476);
  mass_drill.tare(); 

  mass_hd.begin(15, 2);
  mass_hd.set_scale(-395.606476);
  mass_hd.tare(); 

  dust->init();

  Serial.begin(115200);
  
}

void loop() {
  cosco.receive(servo_cam, servo_drill);

  // below is a test to send dust data every 2 seconds
  static uint32_t last_send_dust = 0;
  static uint32_t last_send_mass = 0;
  static uint8_t mass_to_send = 0;

  if (millis() - last_send_dust >= 2000) {
    if(dust->is_alive()){
      DustData dust_packet;
      dust->loop(&dust_packet);
      cosco.sendDustDataPacket(&dust_packet);
      last_send_dust = millis();
    }
  }

  if(millis() - last_send_mass >= 500){
    switch(mass_to_send){
      case 0:
        mass_to_send = 1;
        if(mass_drill.is_ready()){
        float reading = mass_drill.get_units(20); 
        MassPacket drill = {
          MassDrill_ID,
          reading
        };
        cosco.sendMassPacket(&drill, MassDrill_ID);
      }
      break;

      case 1:
        mass_to_send = 0;
        if(mass_hd.is_ready()){
          float reading = mass_hd.get_units(20); 
          MassPacket hd = {
            MassHD_ID,
            reading
          };
          cosco.sendMassPacket(&hd, MassHD_ID);
        }
      break;

      default:
        break;
    }
  }


}