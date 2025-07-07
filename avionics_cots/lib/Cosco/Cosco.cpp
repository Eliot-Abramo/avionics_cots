/**
 * @file Cosco.cpp
 * @author Eliot Abramo
 * @brief Dual-bus comms for ESP32-S3: UART + SPI-slave
 * 
 * (Admire the wonderful drawing, I spent 1h on the plane trying to do this.)
 *
 *   ───────────────   framed messages    ───────────────    queued DMA    ─────────────── 
 *  │  Application  |<───────────────────>│  spiProto     │<──────────────>│  ESP32SPISlave│
 *   ───────────────                     framed <---------- raw bytes -----> DMA / pins   │
 *                                        CRC, IDs                         (your library) │
 *                                                                                        │
 *  This file is the “glue” in the middle.                                                │
 *  ----------------------------------------------------------------------------------- 
*/
#include "Cosco.hpp"
#include <Wire.h>
#include <Arduino.h>
#include <ESP32SPISlave.h>
#include <SPI.h>          
#include <cstring>
#include <algorithm>
#include <SerialProtocol.hpp>
#include <packet_id.hpp>
#include <packet_definition.hpp>
#include "SPISlaveProtocol.hpp"

constexpr std::size_t kPayload = 64;   // protocol max payload
constexpr uint32_t    kUartBaud = 115200;
constexpr std::size_t kSpiBufLen = 256;   //must be multiple of 4

static SerialProtocol<kPayload> uartProto(Serial);
static SPISlaveProtocol<kPayload> spiProto;          
static ESP32SPISlave spiSlave;

static_assert((kSpiBufLen & 3) == 0,  "kSpiBufLen must be 4-byte aligned");

/**************** DMA buffers for one SPI transaction ****************/
static uint8_t spiTxBuf[kSpiBufLen] = { 0xFF };
static uint8_t spiRxBuf[kSpiBufLen] = { 0   };

/**************** SPI-slave  pins  (VSPI default) ****************/
constexpr int PIN_MOSI = 11;  
constexpr int PIN_MISO = 13; 
constexpr int PIN_SCLK = 12;
constexpr int PIN_SS   = 10;

Cosco::Cosco(){
    Serial.begin(kUartBaud);

    spiSlave.setDataMode(SPI_MODE0);
    spiSlave.setQueueSize(1); // one DMA buffer at a time
    
    spiSlave.begin(VSPI, PIN_SCLK, PIN_MISO, PIN_MOSI, PIN_SS);
}

Cosco::~Cosco() { 
    spiSlave.end(); 
}

void Cosco::sendMassPacket(MassPacket* pkt, uint8_t ID) {
    uartProto.send(ID, pkt, sizeof(MassPacket));
    spiProto.send(ID, pkt, sizeof(MassPacket));

}
void Cosco::sendHeartbeat(){
    static uint32_t last_heartbeat = 0;
    if (millis() - last_heartbeat >= 500) { // every 1 s
        uint8_t dummy = 10;
        uartProto.send(Heartbeat_ID, &dummy, 1); 
        spiProto.send(Heartbeat_ID, &dummy, 1); 
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
    uartProto.send(DustData_ID, pkt, sizeof(DustData));
    spiProto.send(DustData_ID, pkt, sizeof(DustData));
}

template<class FrameT>
static Change handleFrame(const FrameT& f, Servo_Driver* cam, Servo_Driver* drill) {
    switch (f.id) {
        case ServoCam_ID:
            if (f.length == sizeof(ServoRequest)) {
                const auto& req = *reinterpret_cast<const ServoRequest*>(f.payload.data());
                cam->set_request(req);
                cam->handle_servo();
            }
            break;

        case ServoDrill_ID:
            if (f.length == sizeof(ServoRequest)) {
                const auto& req = *reinterpret_cast<const ServoRequest*>(f.payload.data());
                drill->set_request(req);
                drill->handle_servo();
            }
            break;

        case MassDrill_Request_ID:
            if (f.length == sizeof(MassRequestDrill)) {
                const auto& req = *reinterpret_cast<const MassRequestDrill*>(f.payload.data());
                return { MassDrill_Request_ID, req.tare, req.scale };
            }
            break;

        case MassHD_Request_ID:
            if (f.length == sizeof(MassRequestHD)) {
                const auto& req = *reinterpret_cast<const MassRequestHD*>(f.payload.data());
                return { MassHD_Request_ID, req.tare, req.scale };
            }
            break;

        default:
            break;
    }
    return {0,0,0};
}

// I have yet to find the words to describe this, it's not tested so I won't yet bother
// but in one sentence: 
// clean out the last SPI transaction + prep the next one.
static Change serviceSpi(Servo_Driver* servo_cam, Servo_Driver* servo_drill) {
    Change change { 0, 0, 0 };

    /**************** Get completed transactions ****************/
    while (spiSlave.numTransactionsCompleted() > 0) {
        const std::size_t rxBytes = std::min<std::size_t>(spiSlave.numBytesReceived(), kSpiBufLen);

        for (std::size_t i = 0; i < rxBytes; ++i) {
            if (spiProto.processByte(spiRxBuf[i])) {
                Change c = handleFrame(spiProto.frame(), servo_cam, servo_drill);
                if (c.id) change = c;         // keep the last non-zero change
            }
        }
    }

    /**************** If idle, queue the next DMA transaction ****************/
    if (spiSlave.numTransactionsInFlight() == 0) {
        // pull up to kSpiBufLen bytes waiting to go out
        std::size_t txCount = 0;
        uint8_t b;
        while (txCount < kSpiBufLen && spiProto.getNextTxByte(b)) {
            spiTxBuf[txCount++] = b;
        }

        // pad with 0xFF and ensure 4-byte alignment
        std::size_t txSizeAligned = txCount ? ( (txCount + 3) & ~3 ) : 4;
        std::fill_n(&spiTxBuf[txCount], txSizeAligned - txCount, 0xFF);

        // queue & trigger a non-blocking transaction
        spiSlave.queue(spiTxBuf, spiRxBuf, txSizeAligned);
        spiSlave.trigger();        // returns immediately
    }

    return change;
}

Change Cosco::receive(Servo_Driver* cam, Servo_Driver* drill) {
    Change spiChange = serviceSpi(cam, drill);
    if (spiChange.id) return spiChange;

    while (Serial.available()) {
        if (uartProto.processByte((uint8_t)Serial.read())) {
            Change c = handleFrame(uartProto.frame(), cam, drill);
            if (c.id) return c;
        }
    }

    return { 0, 0, 0 };
}
