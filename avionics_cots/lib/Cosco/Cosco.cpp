/**
 * @file Cosco.cpp
 * @author Eliot Abramo
*/
#include "Cosco.hpp"
#include <Wire.h>
#include <Arduino.h>
#include <SerialProtocol.hpp>
#include <packet_id.hpp>
#include <packet_definition.hpp>

static SerialProtocol<128> proto(Serial);


Cosco::Cosco()
{
    Serial.begin(115200);
}

Cosco::~Cosco(){}

void Cosco::sendMassPacket(MassPacket* pkt, uint8_t ID) {
    proto.send(ID, pkt, sizeof(MassPacket));
}

void Cosco::sendHeartbeat(){
    static uint32_t last_heartbeat = 0;
    if (millis() - last_heartbeat >= 500) {               // every 1 s
        uint8_t dummy = 10;
        proto.send(Heartbeat_ID, &dummy, 1); 
        last_heartbeat = millis();
    }
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
    proto.send(DustData_ID, pkt, sizeof(DustData));
}

Change Cosco::receive(Servo_Driver* servo_cam, Servo_Driver* servo_drill) {
    while (Serial.available()) {
        if (proto.processByte(Serial.read())) {
            const auto &f = proto.frame();
            switch (f.id) {
                case ServoCam_ID:
                    if (f.length == sizeof(ServoRequest)) {
                        const ServoRequest &req = *reinterpret_cast<const ServoRequest*>(f.payload.data());
                        servo_cam->set_request(req);
                        servo_cam->handle_servo();
                    }
                    break;
                case ServoDrill_ID:
                    if (f.length == sizeof(ServoRequest)) {
                        const ServoRequest &req = *reinterpret_cast<const ServoRequest*>(f.payload.data());
                        servo_drill->set_request(req);
                        servo_drill->handle_servo();
                    }
                    break;

                case MassDrill_Request_ID:
                    if (f.length == sizeof(MassRequestDrill)) {
                        const MassRequestDrill &req = *reinterpret_cast<const MassRequestDrill*>(f.payload.data());
                        Change changeDrill = {MassDrill_Request_ID, req.tare, req.scale};
                        return changeDrill;
                    }
                    break;

                case MassHD_Request_ID:
                    if (f.length == sizeof(MassRequestHD)) {
                        const MassRequestHD &req = *reinterpret_cast<const MassRequestHD*>(f.payload.data());
                        Change changeHD = {MassHD_Request_ID, req.tare, req.scale};
                        return changeHD;
                    }
                    break;

                default:
                    break;
            }
        }
    }
    return Change{0, 0, 0};
}