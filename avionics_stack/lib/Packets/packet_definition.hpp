/** 
 * @file packet_definition.hpp 
 * @author Eliot Abramo 
*/ 

#ifndef PACKET_DEFINITION_H
#define PACKET_DEFINITION_H

#include <iostream>
#include <packet_id.hpp>

struct LEDMessage {
    uint8_t system;
    uint8_t state;
};

struct ServoRequest {
    uint8_t id;
    int32_t increment;
    bool zero_in;
};

struct MassRequestHD {
    bool tare;
    float scale;
};

struct BMS {
    std::string status;
    float v_bat;
    float current;
};

struct MassRequestDrill {
    bool tare;
    float scale;
};

struct DustData {
    uint16_t pm1_0_std;
    uint16_t pm2_5_std;
    uint16_t pm10_std;
    uint16_t pm1_0_atm;
    uint16_t pm2_5_atm;
    uint16_t pm10_atm;
    uint16_t num_particles_0_3;
    uint16_t num_particles_0_5;
    uint16_t num_particles_1_0;
    uint16_t num_particles_2_5;
    uint16_t num_particles_5_0;
    uint16_t num_particles_10;
};

struct Heartbeat {
    uint8_t dummy;
};

struct FourInOne {
    uint16_t id;
    float temperature;
    float humidity;
    float conductivity;
    float ph;
};

struct MassPacket {
    uint8_t id;
    float mass;
};

#endif /* PACKET_DEFINITION_H */