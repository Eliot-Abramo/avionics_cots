
/**
 * @file SPISlaveProtocol.hpp
 * @author Eliot Abramo
 */

/**************** Quick note on this vs SerialProtocol. ****************************
 * Mirrors SerialProtocol framing so the avionics pipeline remains unchanged.
 * 
 * So why is this a separate class?
 * 
 * Because SPI-slave can't push data out — it has to **wait** for the
 * master to clock it. That means instead of writing to a stream, we
 * queue the bytes into a tiny ring buffer and hand them out one-by-one
 * as the master clocks in data.
 * 
 * Think of this as SerialProtocol, but flipped around: it still parses
 * exactly the same thing, just watches from a ring buffer instead of
 * a UART driver.
 *
 **************************************************************************************/

/*************************************************** How to use *************************************************************
 * 1. Start by creating an instance. No arguments needed — this protocol class is agnostic of hardware.
 * 
 *  SPISlaveProtocol<MAX SIZE> _name;    <-->    SPISlaveProtocol<128> spi;
 *
 * 2. Send your packet (TX). Since SPI-slave can't push data, this will queue the frame into an internal ring buffer.
 * The master will pull bytes out when it clocks the SPI lines.
 *
 * spi.send(PacketId, &pkt, sizeof(pkt));
 *
 * 3. Receive packets. This part looks just like SerialProtocol — you feed the bytes in one by one, as they come off SPI.
 * Usually this happens from your SPI slave ISR or polling loop (depending on how your driver works).
 *
 * while (incoming()) { <-- from your SPI input handler
 *   uint8_t b = getNextByteFromSpi(); <-- your board-specific function (can be different, for ESP32 go look at Cosco)
 *   if (spi.processByte(b)) {
 *       handleFrame(spi.frame()); <-- this is where the stuff happens
 *   }
 * }
 *
 * 
 * 4. To transmit bytes, your SPI DMA logic should call:
 *
 * uint8_t b;
 * if (spi.getNextTxByte(b)) {
 *     pushToSpiOutBuffer(b);                     
 * }
 *
 * I'll make the same promise here as in SerialProtocol. Yes this is complicated, yes I have a loooootttt of protections.
 * But I swear this works well. Scouts honor.
 *****************************************************************************************************************************/


#ifndef SPI_SLAVE_PROTOCOL_HPP
#define SPI_SLAVE_PROTOCOL_HPP

#include <Arduino.h>
#include <array>
#include <cstdint>

/***************************** tiny ring buffer *****************************
 * TX buffer for SPI-slave: master clocks out byte-by-byte, so we queue
 * bytes ahead of time in here. Must be power-of-two for modulo math (just 
 * to make the edge cases easier to calculate, this code is complicated enough).
 ***************************************************************************/
template <std::size_t CapacityPow2>
class ByteQueue {
    // Ensures that you have a a power of two for queue size
    static_assert((CapacityPow2 & (CapacityPow2 - 1)) == 0, "Capacity must be a power of two");

public:
    bool push(uint8_t b){
        std::size_t next = (head_ + 1) & mask_;
        if (next == tail_)
            return false; // full
        buf_[head_] = b;
        head_ = next;
        return true;
    }

    bool pop(uint8_t &b){
        if (head_ == tail_)
            return false; // empty
        b = buf_[tail_];
        tail_ = (tail_ + 1) & mask_;
        return true;
    }

    bool empty() const { 
        return head_ == tail_; 
    }

private:
    static constexpr std::size_t mask_ = CapacityPow2 - 1;
    std::array<uint8_t, CapacityPow2> buf_{};
    volatile std::size_t head_ = 0;
    volatile std::size_t tail_ = 0;
};

/***************************** SPI-slave framed protocol *****************************
 * Same protocol as SerialProtocol but adapted for the SPI-slave case.
 *
 * MaxPayload     →  payload size cap
 * TxQueueBytes   →  how much data we can queue for the master to clock out
 *************************************************************************************/
template <std::size_t MaxPayload, std::size_t TxQueueBytes = (MaxPayload + 6) * 2> // ~2 frames

class SPISlaveProtocol {
public:
    struct Frame{
        uint8_t id;
        uint16_t length;
        std::array<uint8_t, MaxPayload> payload;
    };

    /** Queue a packet to be sent out to the master (TX ring).
     *  Still have to call getNextTxByte() when the master clocks it.
     */
    bool send(uint8_t id, const void *payload, uint16_t len) {
        if (len == 0 || len > MaxPayload) return false;

        uint16_t frameLen = len + 1;
        uint16_t crc = crc16(id, static_cast<const uint8_t *>(payload), len);

        noInterrupts();
        bool ok = tx_.push(kStx1) && tx_.push(kStx2) &&
                  push16(frameLen) && tx_.push(id) &&
                  pushBytes(static_cast<const uint8_t *>(payload), len) &&
                  push16(crc);
        interrupts();
        return ok;
    }

    // Same logic as SerialProtcol.hpp
    bool processByte(uint8_t b) {
        switch (state_) {
        case State::Stx1:
            state_ = (b == kStx1) ? State::Stx2 : State::Stx1;
            break;
        case State::Stx2:
            state_ = (b == kStx2) ? State::LenLo : State::Stx1;
            break;
        case State::LenLo:
            len_ = b;
            state_ = State::LenHi;
            break;
        case State::LenHi:
            len_ |= uint16_t(b) << 8;
            if (len_ == 0 || len_ > MaxPayload + 1) {
                reset(); 
                break;
            }
            crc_ = 0xFFFF;
            bytes_ = 0;
            state_ = State::Id;
            break;
        case State::Id:
            frame_.id = b;
            crc_ = updateCrc(crc_, b);
            state_ = (len_ == 1) ? State::CrcLo : State::Payload;
            break;
        case State::Payload:
            frame_.payload[bytes_++] = b;
            crc_ = updateCrc(crc_, b);
            if (bytes_ >= len_ - 1)
                state_ = State::CrcLo;
            break;
        case State::CrcLo:
            crcRead_ = b;
            state_ = State::CrcHi;
            break;
        case State::CrcHi:
            crcRead_ |= uint16_t(b) << 8;
            if (crcRead_ == crc_){
                frame_.length = len_ - 1;
                complete_ = true;
            }
                reset();  // always reset, even on CRC fail
            break;
        }
        return complete_;
    }

    /** Like the name suggests, call this when valid frame just completed */
    bool frameAvailable() {
        bool c = complete_;
        complete_ = false;
        return c;
    }

    /** Grab the last completed frame. Only valid if frameAvailable() returned true. */
    const Frame &frame() const { return frame_; }

    /* called by SPI driver when master clocks data out */
    bool getNextTxByte(uint8_t &b) { return tx_.pop(b); }

private:
    enum class State : uint8_t {
        Stx1, Stx2, LenLo, LenHi, Id, Payload, CrcLo, CrcHi
    };
    
    static constexpr uint8_t kStx1 = 0xA5;
    static constexpr uint8_t kStx2 = 0x5A;

    // Pick next power-of-two buffer capacity ≥ TxQueueBytes
    static constexpr std::size_t kTxCap = 1ULL << (static_cast<std::size_t>(ceil(log2(TxQueueBytes))));

    ByteQueue<kTxCap> tx_; // our TX ring buffer

    Frame frame_{};
    State state_ = State::Stx1;
    uint16_t len_ = 0, bytes_ = 0, crcRead_ = 0, crc_ = 0;
    bool complete_ = false;

    /* Reset parser to sync state */
    void reset(){
        state_ = State::Stx1;
        len_ = bytes_ = crcRead_ = 0;
    }

    bool push8(uint8_t v) { return tx_.push(v); }
    
    bool push16(uint16_t v) { return push8(uint8_t(v & 0xFF)) && push8(uint8_t(v >> 8)); }
    
    bool pushBytes(const uint8_t *p, uint16_t n){
        for (uint16_t i = 0; i < n; ++i)
            if (!push8(p[i]))
                return false;
        return true;
    }

    /* CRC-16/X-25, same as SerialProtocol */
    static uint16_t updateCrc(uint16_t c, uint8_t b) {
        c ^= b;
        for (uint8_t i = 0; i < 8; ++i)
            c = (c & 1) ? (c >> 1) ^ 0xA001 : c >> 1;
        return c;
    }
    static uint16_t crc16(uint8_t id, const uint8_t *d, uint16_t n) {
        uint16_t c = 0xFFFF;
        c = updateCrc(c, id);
        for (uint16_t i = 0; i < n; ++i)
            c = updateCrc(c, d[i]);
        return c;
    }
};

#endif /* SPI_SLAVE_PROTOCOL_HPP */
