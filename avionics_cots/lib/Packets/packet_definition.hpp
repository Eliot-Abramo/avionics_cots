#pragma once

#include <iostream>

struct LEDMessage {
    uint8_t low;
    uint8_t high;
    uint8_t system;
    uint8_t mode;
};

struct ScFSMStatusDrill {
    int8 mode;
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
    float32[4] min_duty;
    float32[4] max_duty;
    float32[4] min_angles;
    float32[4] max_angles;
    bool remote_command;
    bool set_min_duty;
    bool set_max_duty;
    bool set_min_angles;
    bool set_max_angles;
};

struct Motorcmds {
    string modedeplacement;
    string info;
    float64[4] drive;
    float64[4] steer;
};

struct ServoConfigResponse {
    uint16_t id;
    float32[4] min_duty;
    float32[4] max_duty;
    float32[4] min_angles;
    float32[4] max_angles;
    bool remote_command;
    bool set_min_duty;
    bool set_max_duty;
    bool set_min_angles;
    bool set_max_angles;
    bool success;
};

struct FourInOne {
    uint16_t id;
    float32 temperature;
    float32 moisture;
    float32 conductivity;
    float32 ph;
};

struct ServoResponse {
    uint16_t id;
    uint8_t channel;
    float32 angle;
    bool success;
};

struct MassArray {
    uint16_t id;
    float32[4] mass;
};

struct ScMotorCmds {
    bool send_pos;
    bool send_vel;
    float64 position;
    float64 velocity;
};

struct Wheelstatus {
    float64[4] data;
    bool[8] state;
    float64[8] current;
};

struct MassConfigRequestJetson {
    uint16_t destination_id;
    float32[4] offset;
    float32[4] scale;
    float32 alpha;
    bool[4] enabled_channels;
    bool remote_command;
    bool set_offset;
    bool set_scale;
    bool set_alpha;
    bool set_channels_status;
};

struct NPK {
    uint16_t id;
    uint16_t nitrogen;
    uint16_t phosphorus;
    uint16_t potassium;
};

struct MassConfigResponse {
    uint16_t id;
    float32[4] offset;
    float32[4] scale;
    float32 alpha;
    bool[4] enabled_channels;
    bool remote_command;
    bool set_offset;
    bool set_scale;
    bool set_alpha;
    bool set_channels_status;
    bool success;
};

struct ScDrillCmds {
    string mode;
    bool send_parameter;
    float64 rotation_speed;
};

struct NavFeedback {
    float32 distance_remaining;
    string status;
};

struct MotorStatus {
    bool[8] state;
    float64[8] current;
    float64[8] average_current;
    float64[4] position;
    float64[4] velocity;
    bool[8] fault_state;
};

struct AngleArray {
    uint16_t id;
    float32[4] angles;
};

struct MassConfigRequestMCU {
    uint16_t id;
    bool req_offset;
    bool req_scale;
    bool req_alpha;
    bool req_channels_status;
};

struct ScMotorStatus {
    float64 encoder;
    float64 distance;
    float64 trans_current;
    float64 screw_current;
    float64 vel;
    bool motor_screw;
    bool motor_trans;
};

struct Statussteering {
    int64[4] steer;
};

struct NodeStateArray {
    bool[] node_state;
};

struct MassCalibScale {
    uint16_t destination_id;
    uint8_t channel;
    float32 expected_weight;
};

struct ServoRequest {
    float32 angle;
};

struct LEDResponse {
    bool success;
};

struct MassCalibOffset {
    uint16_t destination_id;
    uint8_t channel;
};

