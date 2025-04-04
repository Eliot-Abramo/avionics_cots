/**
 * @file Cosco.cpp
 * @author Eliot Abramo
*/
#include "Cosco.hpp"
#include <Wire.h>
#include <Arduino.h>
#include <packet_id.hpp>
#include <packet_definition.hpp>

extern MassConfigRequestPacket latest_mass_config_request;
extern MassConfigResponsePacket latest_mass_config_response;

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

void Cosco::sendMassConfigPacket(MassConfigPacket *configPacket)
{
    // Serialize and send MassConfigPacket
    uint8_t packetBuffer[sizeof(MassConfigPacket) + 1];
    packetBuffer[0] = MassConfig_ID; 
    memcpy(packetBuffer + 1, configPacket, sizeof(MassConfigPacket));
    Serial.write(packetBuffer, sizeof(MassConfigPacket));
}

void Cosco::sendMassConfigRequestPacket(MassConfigRequestPacket* pkt) {
    uint8_t buffer[sizeof(MassConfigRequestPacket) + 1];
    buffer[0] = MassConfigRequest_ID;
    memcpy(buffer + 1, pkt, sizeof(MassConfigRequestPacket));
    Serial.write(buffer, sizeof(buffer));
    Serial.flush();       // make sure it's all sent
    delay(5);             // give host time to react
}

void Cosco::sendMassConfigResponsePacket(MassConfigResponsePacket* pkt) {
    uint8_t buffer[sizeof(MassConfigResponsePacket) + 1];
    buffer[0] = MassConfigResponse_ID;
    memcpy(buffer + 1, pkt, sizeof(MassConfigResponsePacket));
    Serial.write(buffer, sizeof(buffer));
    Serial.flush();       // make sure it's all sent
    delay(5);             // give host time to react
}

void Cosco::sendServoRequestPacket(ServoRequest* pkt) {
    uint8_t buffer[sizeof(ServoRequest) + 1];
    buffer[0] = ServoConfigRequest_ID;
    memcpy(buffer + 1, pkt, sizeof(ServoRequest));
    Serial.write(buffer, sizeof(buffer));
    Serial.flush();       // make sure it's all sent
    delay(5);             // give host time to react
}

void Cosco::sendServoResponsePacket(ServoResponse* pkt) {
    uint8_t buffer[sizeof(ServoResponse) + 1];
    buffer[0] = ServoConfigResponse_ID;
    memcpy(buffer + 1, pkt, sizeof(ServoResponse));
    Serial.write(buffer, sizeof(buffer));
    Serial.flush();       // make sure it's all sent
    delay(5);             // give host time to react
}

void Cosco::sendDustDataPacket(DustData* dust_packet) {
    uint8_t buffer[sizeof(DustData) + 1];
    buffer[0] = DustData_ID; // <<< PREPEND THE ID
    memcpy(buffer + 1, dust_packet, sizeof(DustData));
    Serial.write(buffer, sizeof(buffer));
}


void Cosco::sendMassDataPacket(MassData *responsePacket)
{
    // Serialize and send sendMassDataPacket
    uint8_t packetBuffer[sizeof(MassData) + 1];
    packetBuffer[0] = MassData_ID;
    memcpy(packetBuffer, responsePacket, sizeof(MassData));
    Serial.write(packetBuffer, sizeof(MassData));
}

void Cosco::receive(Servo_Driver* servo_cam) {
    if (Serial.available() < 1) return;

    uint8_t packet_id = Serial.read();

    switch (packet_id) {
        // case MassConfigRequest_ID:
        //     if (Serial.available() >= sizeof(MassConfigRequestPacket)) {
        //         Serial.readBytes(reinterpret_cast<char*>(&latest_mass_config_request),
        //                          sizeof(MassConfigRequestPacket));
        //         sendMassConfigRequestPacket(&latest_mass_config_request);  // Echo back
        //     }
        //     break;

        // case MassConfigResponse_ID:
        //     if (Serial.available() >= sizeof(MassConfigResponsePacket)) {
        //         Serial.readBytes(reinterpret_cast<char*>(&latest_mass_config_response),
        //                          sizeof(MassConfigResponsePacket));
        //         sendMassConfigResponsePacket(&latest_mass_config_response);  // Echo back
        //     }
        //     break;

        case ServoConfigRequest_ID:
            if (Serial.available() >= sizeof(ServoRequest)) {
                ServoRequest request;
                Serial.readBytes(reinterpret_cast<char*>(&request), sizeof(ServoRequest));
                
                // request.id++;
                // request.increment--;
                // sendServoRequestPacket(&request);
                // request.id = 0;
                // request.increment = 20;
                // request.zero_in = false;
                servo_cam->set_request(request);
                servo_cam->handle_servo();
                
                sendServoResponsePacket(servo_cam->get_response());
                break;
            }

        default:
            break;
    }
}