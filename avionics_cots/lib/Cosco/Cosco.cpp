/**
 * @file Cosco.cpp
 * @author Eliot Abramo
*/
#include "Cosco.hpp"
#include <Wire.h>
#include <Arduino.h>
#include <packet_id.hpp>
#include <packet_definition.hpp>

Cosco::Cosco()
{
    Serial.begin(115200);
}

Cosco::~Cosco(){}

void Cosco::sendMassPacket(MassPacket* pkt, uint8_t ID) {
    uint8_t buffer[sizeof(MassPacket) + 1];
    buffer[0] = ID;
    memcpy(buffer + 1, pkt, sizeof(MassPacket));
    Serial.write(buffer, sizeof(buffer));
}

// void Cosco::sendServoRequestPacket(ServoRequest* pkt) {
//     uint8_t buffer[sizeof(ServoRequest) + 1];
//     buffer[0] = ServoConfigRequest_ID;
//     memcpy(buffer + 1, pkt, sizeof(ServoRequest));
//     Serial.write(buffer, sizeof(buffer)); 
//     Serial.flush();       // make sure it's all sent
//     delay(5);             // give host time to react
// }

// void Cosco::sendServoCamResponse(ServoResponse* pkt) {
//     uint8_t buffer[sizeof(ServoResponse) + 1];
//     buffer[0] = ServoCam_ID;
//     memcpy(buffer + 1, pkt, sizeof(ServoResponse));
//     Serial.write(buffer, sizeof(buffer));
// }

// void Cosco::sendServoDrillResponse(ServoResponse* pkt) {
//     uint8_t buffer[sizeof(ServoResponse) + 1];
//     buffer[0] = ServoDrill_ID;
//     memcpy(buffer + 1, pkt, sizeof(ServoResponse));
//     Serial.write(buffer, sizeof(buffer));
// }

void Cosco::sendDustDataPacket(DustData* pkt) {
    uint8_t buffer[sizeof(DustData) + 1];
    buffer[0] = DustData_ID;
    memcpy(buffer + 1, pkt, sizeof(DustData));
    Serial.write(buffer, sizeof(buffer));
}

void Cosco::receive(Servo_Driver* servo_cam, Servo_Driver* servo_drill) {
    if (Serial.available() < 1) return;

    uint8_t packet_id = Serial.read();

    switch (packet_id) {
        case ServoCam_ID:
            if (Serial.available() >= sizeof(ServoRequest)) {
                ServoRequest request;
                Serial.readBytes(reinterpret_cast<char*>(&request), sizeof(ServoRequest));                
                servo_cam->set_request(request);
                servo_cam->handle_servo();   
                // sendServoCamResponse(servo_cam->get_response());
                break;
            }

        case ServoDrill_ID:
            if (Serial.available() >= sizeof(ServoRequest)) {
                ServoRequest request;
                Serial.readBytes(reinterpret_cast<char*>(&request), sizeof(ServoRequest));                
                servo_drill->set_request(request);
                servo_drill->handle_servo();
                // sendServoDrillResponse(servo_drill->get_response());
                break;
            }

        default:
            break;
    }

    // mass_hd->set_scale(100);
    // printf("Set: %d", mass_hd->get_scale());
}