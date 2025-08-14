/**
 * @file SerialProtocol.hpp
 * @author Eliot Abramo
 * @brief 
 * @date 2025-07-03
 */
#ifndef SERIAL_PROTOCOL_HPP
#define SERIAL_PROTOCOL_HPP

#include <Arduino.h>
#include <array>
#include <cstdint>

/*************************************************** How to use *************************************************************
 * 1. Seems trivial but important, just create an instance. As an argument you can pass anything that 
 * inherits from c++ Stream library.
 * 
 * SerialProtocol<MAX SIZE> _name(Something that inherits from Stream) <--> SerialProtocol<128> proto(Serial);
 * 
 * 2. Send your packet (TX). You just use the send() function with your packet as an argument (which has to be less than 
 * the max you defined).
 * 
 * proto.send(PacketId, &pkt, sizeof(pkt));
 * 
 * 3. Receive packet. Now this might be clearer if you look at ERC_EL_Broco which is where I do this. But essentially, just 
 * feed the bytes one at a time.
 * 
 * while (Serial.available()) {
 *  if (proto.processByte(Serial.read())) {
 *      handleFrame(proto.frame());  <--- this is where we read stuff
 *  }
 * }
 * 
 * That’s it.  No dynamic allocation, no memcpy, no circular buffers.
 * (I swear this works quite well, scouts honor).
*****************************************************************************************************************************/

/*************************************************** Why custom? *************************************************************
 * Cause since we are reading byte by byte the data, I want to control everything on the bus. Standard methods weren't precise 
 * enough but also more importantly not robust enough. Packet synchronization was a nightmare and I hated it. So this was born.
*****************************************************************************************************************************/


template <std::size_t MaxPayload>
class SerialProtocol {
public:
    struct Frame {
        uint8_t id;                              // Packet ID (router key)
        uint16_t length;                         // Payload size, HAS TO BE less than MaxPayload (def above)
        std::array<uint8_t, MaxPayload> payload; // Raw payload bytes
    };

    /* With this you can plug in any Arduino Stream (HardwareSerial, Wire, …) which I find very cool and also very flexible */
    explicit SerialProtocol(Stream &stream) : s_(stream) {}

    /****************************** Send ******************************
     * @brief Push an already-formed payload onto the wire.
     * @param id: application-level packet identification,these are definied in lib>Packets>packet_id.hpp 
     * @param payload: raw bytes to send 
     * @param len: it's in the name, length of packet(len ≥ 1, ≤ MaxPayload)
     *
     * Safety guards:
     *   – oversized or zero-length payloads are silently dropped.
     *   – send() is synchronous; call from a low-duty loop or wrap
     *     it in a task if you need async.
     */
    void send(uint8_t id, const void *payload, uint16_t len) {
        if (len > MaxPayload || len == 0) return;   // drop oversized/empty packets
        /* Start-of-frame markers as explained in comment of proccessByte*/
        write8(kStx1);
        write8(kStx2);

        /* Length = id + payload = 1 + payload */
        write16(static_cast<uint16_t>(len + 1));

        /* ID byte */
        write8(id);

        /* Raw payload */
        const uint8_t* p = static_cast<const uint8_t*>(payload);
        s_.write(p, len);

        /* CRC (calculated over ID+payload) */
        write16(crc16(id, p, len));

        /* Make sure everything actually leaves the HW FIFO. Made for some wierd debugging lol */
        s_.flush();
    }
    
    /** Ok, I am going to do my best to explain how this works but I understand that it can be daunting 
     * by looking at it. If you have any questions feel free to send me a message on slack @Eliot Abramo. 
     * 
     * The idea: This feeds a single byte into the protocol state machine. It returns true if a complete, 
     * valid frame has been received. This provides an aggressive filter for any noise that may appear on 
     * the serial line, which has many advantages as we are certain that the pipeline won't crash. However,
     * big drawback, it is very severe, so packets have to be perfect or they are rejected.
     *
     * How it works:
     * - The function keeps track of where we are in the frame (start bytes, length, id, payload, CRC).
     * I chose to make packets look like this (if you really want to know why, google or ask me):
     * 
     *   +------------+------------+----------------+----------+----------------+---------+
     *  | 0xA5 (STX) | 0x5A       | uint16 len     | uint8 id | payload[len-1] | CRC16   |
     *  +------------+------------+----------------+----------+----------------+---------+
     *  - len counts *id + payload* (so min len=1). CRC is X25/Modbus (poly 0xA001).
     * 
     * - Start bytes 0xA5, 0x5A were chosen because they're rare in ASCII streams. They quite literally represent the 
     * symbol for the Yen (0xA5) and then I did 0x5A cause it's the inverse so fits well. Unless the Yen becomes a unit
     * of measurement for an actuator, we should be good. (You could have put whatever you want here btw)
     * 
     * - It then waits for the two start bytes (0xA5, 0x5A) to sync up ().
     * 
     * - Then it reads the length (2 bytes, little-endian), checks it's not too big or zero.
     * 
     * - Next, it grabs the packet ID (very important step).
     * 
     * - If there's a payload, it reads that in, otherwise skips to CRC. (CRC <-> check if packet isn't compromise)
     * 
     * - Finally, it reads the CRC (2 bytes) and checks if it matches what we expect.
     * 
     * - If everything checks out, it fills in the frame and returns true.
     * 
     * - If anything goes wrong (bad length, CRC mismatch, etc), it resets and waits for the next frame.
     * 
     */    

     bool processByte(uint8_t b) {
        switch (state_) {
            /**** 0xA5 hunt ****/
            case State::Stx1:
                if (b == kStx1) state_ = State::Stx2;
                break;

            /**** 0x5A confirmation ****/
            case State::Stx2:
                state_ = (b == kStx2) ? State::LenLo : State::Stx1;
                break;

            /**** length low byte ****/
            case State::LenLo:
                len_ = b;
                state_ = State::LenHi;
                break;

            /**** length high byte ****/
            case State::LenHi:
                len_ |= static_cast<uint16_t>(b) << 8;
                // sanity check
                if (len_ == 0 || len_ > MaxPayload + 1) { reset(); break; }
                bytes_ = 0;                     // new payload counter
                state_ = State::Id;
                break;

            /**** packet ID ****/
            case State::Id:
                frame_.id = b;
                bytes_ = 0;
                state_ = (len_ == 1) ? State::CrcLo : State::Payload;
                break;

            /**** payload stream ****/
            case State::Payload:
                frame_.payload[bytes_++] = b;
                if (bytes_ == len_ - 1) state_ = State::CrcLo;
                break;

            /**** CRC16 LSB ****/
            case State::CrcLo:
                crcRead_ = b;
                state_ = State::CrcHi;
                break;

            /**** CRC16 MSB & verdict ****/
            case State::CrcHi:
                crcRead_ |= static_cast<uint16_t>(b) << 8;
                if (crcRead_ == crc16(frame_.id, frame_.payload.data(), len_ - 1)) {
                    frame_.length = len_ - 1;  // strip ID
                    reset();                   // ready for next frame
                    return true;               // success!
                }
                /* CRC mismatch -> drop frame and resync */
                reset();
                break;
        }
        return false;                          // frame not yet finished
    }

    /** Grab the last good frame.
     * Only call RIGHT AFTER processByte() returned true. 
    */
    const Frame& frame() const { return frame_; }

  private:
    /**internal stuff, helps keep track of packet structure. No deeper meaning than (hopefully) more
     * legible code (I swear, I'm trying).
    */
    enum class State : uint8_t { Stx1, Stx2, LenLo, LenHi, Id, Payload, CrcLo, CrcHi };

    static constexpr uint8_t kStx1 = 0xA5; // start token 1
    static constexpr uint8_t kStx2 = 0x5A; // start token 2

    Stream &s_; // wire abstraction
    Frame frame_{}; // rolling RX buffer
    State state_ = State::Stx1; // current parser state
    uint16_t len_ = 0; // expected (id+payload) length
    uint16_t bytes_ = 0; // payload bytes read so far
    uint16_t crcRead_ = 0; // CRC from wire

    /* Reset parser to STX hunt */
    void reset() {
        state_ = State::Stx1;
        len_ = bytes_ = crcRead_ = 0;
    }

    /* One-byte helpers (little-endian) */
    void write8(uint8_t v) {
        s_.write(v); 
    }
    void write16(uint16_t v) {
        s_.write(static_cast<uint8_t>(v & 0xFF));
        s_.write(static_cast<uint8_t>((v >> 8) & 0xFF));
    }

    static uint16_t updateCrc(uint16_t crc, uint8_t b) {
        crc ^= b;
        for (uint8_t i = 0; i < 8; ++i) crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
        return crc;
    }
    static uint16_t crc16(uint8_t id, const uint8_t *data, uint16_t len) {
        uint16_t crc = 0xFFFF;
        crc = updateCrc(crc, id);
        for (uint16_t i = 0; i < len; ++i) crc = updateCrc(crc, data[i]);
        return crc;
    }
};

#endif /* SERIAL_PROTOCOL_HPP */