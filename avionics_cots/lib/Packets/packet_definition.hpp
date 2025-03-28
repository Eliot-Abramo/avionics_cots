/** 
 * @file packet_definition.hpp 
 * @author Eliot Abramo 
*/ 

#ifndef PACKET_DEFINITION_H
#define PACKET_DEFINITION_H

#include <iostream>
#include <packet_id.hpp>

struct __packed AngleArray {
    uint16_t id;
    float angles[4];
};

struct __packed FourInOne {
    uint16_t id;
    float temperature;
    float moisture;
    float conductivity;
    float ph;
};

struct __packed LEDMessage {
    uint8_t low;
    uint8_t high;
    uint8_t system;
    uint8_t mode;
};

struct __packedLEDResponse {
    bool success;
};

struct __packed MassPacket {
    uint16_t id;
    float mass;
};

struct __packed MassCalibPacket{
    uint16_t id;
    float expected_weight; //known weight to use for calibration
    uint32_t num_samples; //number of samples we are going to average to take for calibration
    bool calibrate_offset; //true if we are calibrating offset
    bool calibrate_scale; //true if we are calibrating scale
};

struct __packed MassConfigPacket {
    uint16_t id; 
    float offset; //actual offset value
    float scale; //actual scale value
    bool set_offset; //true if we are setting the offset
    bool set_scale; //true if we are setting the scale
};

struct __packed MassConfigRequestPacket {
    uint16_t id;
    bool req_config; // true if we are requesting configuration
};

struct __packed MassConfigResponsePacket {
    uint16_t id;
    float offset; //current offset value
    float scale; //current scale value
    bool offset_set; // true if we set the offset
    bool scale_set; // true if we set the scale
};

struct __packed NPK {
    uint16_t id;
    uint16_t nitrogen;
    uint16_t phosphorus;
    uint16_t potassium;
};

struct __packed ServoConfigRequestJetson {
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

struct __packed ServoConfigRequestMCU {
    uint16_t id;
    bool req_min_duty;
    bool req_max_duty;
    bool req_min_angles;
    bool req_max_angles;
};

struct __packed ServoConfigResponse {
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

struct __packed ServoRequest {
    float angle;
};

struct __packed ServoResponse {
    uint16_t id;
    uint8_t channel;
    float angle;
    bool success;
};

#endif /* PACKET_DEFINITION_H */