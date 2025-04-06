/**
 * @file Cosco.cpp
 * @author Eliot Abramo
*/
#include "Cosco.hpp"
#include <Wire.h>
#include <Arduino.h>
#include <packet_id.hpp>
#include <packet_definition.hpp>

// Macro de Ilyas
// #define HANDLE_PACKET(packet_type) do {                                         \
//     if (len == sizeof(packet_type)) {                                           \
//         memcpy(packet, buffer + 1, sizeof(packet_type));                        \
//         Serial.x(String(#packet_type) + " packet copied successfully");   \
//     } else {                                                                    \
//         Serial.println("Received data too "                                     \
//                         + String(len > sizeof(packet_type) ? "long" : "short")  \
//                         + " for " + String(#packet_type));                      \
//     }                                                                           \
//     break;                                                                      \
// } while (0)

Cosco::Cosco()
{
    Serial.begin(115200);
    // Serial.println("Serial launched");
}

Cosco::~Cosco(){}

void Cosco::sendMassPacket(MassPacket *responsePacket)
{
    // Serialize and send sendMassDataPacket
    uint8_t packetBuffer[sizeof(MassPacket) + 1];
    packetBuffer[0] = MassData_ID;
    memcpy(packetBuffer, responsePacket, sizeof(MassPacket));
    Serial.write(packetBuffer, sizeof(MassPacket));
}

// void Cosco::sendServoRequestPacket(ServoRequest* pkt) {
//     uint8_t buffer[sizeof(ServoRequest) + 1];
//     buffer[0] = ServoConfigRequest_ID;
//     memcpy(buffer + 1, pkt, sizeof(ServoRequest));
//     Serial.write(buffer, sizeof(buffer));
//     Serial.flush();       // make sure it's all sent
//     delay(5);             // give host time to react
// }

void Cosco::sendServoCamResponse(ServoResponse* pkt) {
    uint8_t buffer[sizeof(ServoResponse) + 1];
    buffer[0] = ServoCam_ID;
    memcpy(buffer + 1, pkt, sizeof(ServoResponse));
    Serial.write(buffer, sizeof(buffer));
}

void Cosco::sendServoDrillResponse(ServoResponse* pkt) {
    uint8_t buffer[sizeof(ServoResponse) + 1];
    buffer[0] = ServoDrill_ID;
    memcpy(buffer + 1, pkt, sizeof(ServoResponse));
    Serial.write(buffer, sizeof(buffer));
}

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
        // case MassData_ID:
        //     if (Serial.available() >= sizeof(MassData_ID)) {
        //         MassPacket request;
        //         Serial.readBytes(reinterpret_cast<char*>(&request), sizeof(MassData_ID));
        //         sendMassPacket(&request);
        //     }
        //     break;

        case ServoCam_ID:
            if (Serial.available() >= sizeof(ServoRequest)) {
                ServoRequest request;
                Serial.readBytes(reinterpret_cast<char*>(&request), sizeof(ServoRequest));                
                servo_cam->set_request(request);
                servo_cam->handle_servo();   
                sendServoCamResponse(servo_cam->get_response());
            }
            break;
        
        case ServoDrill_ID:
            if (Serial.available() >= sizeof(ServoRequest)) {
                ServoRequest request;
                Serial.readBytes(reinterpret_cast<char*>(&request), sizeof(ServoRequest));                
                servo_drill->set_request(request);
                servo_drill->handle_servo();
                sendServoDrillResponse(servo_drill->get_response());
            }
            break;

        default:
            break;
    }
}