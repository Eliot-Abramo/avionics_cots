/** 
 * @file packet_definition.hpp 
 * @author Eliot Abramo 
*/ 

#ifndef PACKET_DEFINITION_H
#define PACKET_DEFINITION_H

#include <iostream>
#include <packet_id.hpp>

struct LEDMessage {
    uint8_t low;
    uint8_t high;
    uint8_t system;
    uint8_t mode;
};

struct ServoConfigRequestMCU {
    uint16_t id;
    bool req_min_duty;
    bool req_max_duty;
    bool req_min_angles;
    bool req_max_angles;
};

struct ServoConfigRequestJetson {
    uint16_t destination_id;
    float min_duty[4];
    float max_duty[4];
    float min_angles[4];
    float max_angles[4];
    bool remote_command;
    bool set_min_duty;
    bool set_max_duty;
    bool set_min_angles;
    bool set_max_angles;
};

struct ServoConfigResponse {
    uint16_t id;
    float min_duty[4];
    float max_duty[4];
    float min_angles[4];
    float max_angles[4];
    bool remote_command;
    bool set_min_duty;
    bool set_max_duty;
    bool set_min_angles;
    bool set_max_angles;
    bool success;
};

struct FourInOne {
    uint16_t id;
    float temperature;
    float moisture;
    float conductivity;
    float ph;
};

struct ServoResponse {
    uint16_t id;
    uint8_t channel;
    float angle;
    bool success;
};

struct MassConfigPacket {
    uint16_t destination_id;
    float offset[4];
    float scale[4];
    float alpha;
    bool enabled_channels[4];
    bool remote_command;
    bool set_offset;
    bool set_scale;
    bool set_alpha;
    bool set_channels_status;
};

struct MassConfigResponsePacket {
    uint16_t id;
    float offset[4];
    float scale[4];
    float alpha;
    bool enabled_channels[4];
    bool remote_command;
    bool set_offset;
    bool set_scale;
    bool set_alpha;
    bool set_channels_status;
    bool success;
};

struct MassArray {
    uint16_t id;
    float mass[4];
};

struct NPK {
    uint16_t id;
    uint16_t nitrogen;
    uint16_t phosphorus;
    uint16_t potassium;
};

struct AngleArray {
    uint16_t id;
    float angles[4];
};

struct MassCalibScale {
    uint16_t destination_id;
    uint8_t channel;
    float expected_weight;
};

struct ServoRequest {
    float angle;
};

struct LEDResponse {
    bool success;
};

struct MassConfigRequestPacket {
    uint16_t id;
    bool req_offset;
    bool req_scale;
    bool req_alpha;
    bool req_channels_status;
};

struct MassCalibOffset {
    uint16_t destination_id;
    uint8_t channel;
};

#endif /* PACKET_DEFINITION_H */