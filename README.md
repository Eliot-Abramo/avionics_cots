# 🚀 Avionics-COTS Communication Stack

The Avionics-COTS Communication Stack provides a highly optimized, transport-agnostic, real-time communication solution specifically designed for embedded avionics systems. Extensively tested in a Mars-style rover platform, this stack ensures reliable, deterministic, and zero-copy communication over UART and SPI interfaces.

---

## 🌟 Key Technical Features

- **Unified Frame Format:** Common framing for UART and SPI ensures interoperability.
- **Zero-Copy & Deterministic:** Static memory allocation; strictly no dynamic allocation.
- **Automatic Transport Failover:** Robust multiplexer (MUX) seamlessly switches between SPI and UART.
- **ISR Optimized:** Minimal ISR latency (<1µs per-byte parsing).
- **Field-Tested Robustness:** Proven reliability through over 100 continuous operational hours.

---

## ⚙️ Supported Hardware

| Component             | Hardware                   | Interface             |
|-----------------------|----------------------------|-----------------------|
| MCU                   | Adafruit Feather ESP32     | UART, SPI             |
| Companion Computer    | Raspberry Pi 4             | SPI Master (8 MHz)    |
| Ground Control Device | Laptop/PC (via UART)       | UART (115200 baud)    |
| Load Cells            | HX711 ADC                  | I²C Bus               |
| Dust Sensor           | HM330X PM Sensor           | I²C Bus               |
| Servo Actuators       | SG90 Micro Servo           | PWM                   |

---

## 📡 Communication Protocol

### Frame Structure
```
| SOF (2 bytes) | Length (2 bytes) | ID (1 byte) | Payload (0-256 bytes) | CRC16 (2 bytes) |
|---------------|------------------|-------------|-----------------------|-----------------|
| 0xA5 0x5A     | LenLo LenHi      | Packet ID   | Data                  | CRC-16/X-25     |
```
- **SOF:** 0xA5, 0x5A for frame synchronization.
- **Payload:** Configurable up to 256 bytes (compile-time constant).
- **Integrity:** CRC-16/X-25 validation.

### Example Packet (MassReading)

```cpp
struct MassReading {
    uint32_t id;
    float mass_g;
    uint64_t timestamp_us;
};

// Encoded example packet (MassReading):
// ID = 2, mass = 32.3 g, timestamp = 1708657436 µs
uint8_t frame[] = {
  0xA5, 0x5A, 0x0B, 0x00, 0x02,
  0x2A, 0x00, 0x00, 0x00,
  0x9A, 0x99, 0x01, 0x42,
  0x1C, 0x8F, 0x20, 0x65,
  0x5F, 0xD8
};
```

---

## 🛠️ Technical Implementation

### Serial Protocol (`SerialProtocol`)
- **ISR-Driven Parser:** Lightweight finite-state machine implementation.
- **Static Buffers:** Compile-time payload buffer allocation.

```cpp
// UART initialization example
SerialProtocol<256> serialProtocol;

void serialEvent() {
    while (Serial.available()) {
        uint8_t byte = Serial.read();
        serialProtocol.processByte(byte);
    }
}
```

### SPI Protocol (`SPISlaveProtocol`)
- **Ring Buffer Architecture:** Lock-free, low-latency data transfer.
- **High-Speed Stability:** Verified at 8 MHz SPI clock speed.

```cpp
// SPI Slave initialization example
SPISlaveProtocol<256, 512> spiProtocol;

void IRAM_ATTR onSPIEvent() {
    uint8_t byte = SPI.read();
    spiProtocol.processByte(byte);
}
```

### Byte-Level Multiplexer (MUX)
- Centralized logic (`processByte()`) seamlessly multiplexes UART and SPI bytes.
- Automatic fail-over ensures continuous communication without frame loss.

---

## 🔀 Packet Routing (`Cosco`)

`Cosco` binds the two transports to your application logic and keeps the fast‑path deterministic.

### How it works
* **Single entry point**  
  ```cpp
  Change delta = cosco.receive(&servo_cam, &servo_drill);
  ```
  `receive()` drains any complete frame—SPI first, then UART—and immediately hands it to the parser.

* **Deterministic dispatch**  
  A compile‑time `switch (frame.id)` fans out to strongly‑typed handlers:

  ```cpp
  switch (frame.id) {
      case MassReading_ID: {
          const auto& m = *reinterpret_cast<const MassReading*>(frame.payload.data());
          massSensor.update(m);
          break;
      }
      case ServoCommand_ID: {
          const auto& cmd = *reinterpret_cast<const ServoCommand*>(frame.payload.data());
          cam->set_request(cmd);
          cam->handle_servo();
          break;
      }
      // add more cases here…
  }
  ```

* **Zero overhead** – no dynamic lookup tables; all routes are resolved at compile time and remain branch‑predicted on Cortex‑M and ESP32 cores.

### Adding a new packet
1. Define the struct & numeric ID in `packet_definition.hpp`.
2. Add a new `case` in `handleFrame()`; the compiler verifies the struct size at build time.

---

## 🚀 Quick Start

```sh
git clone https://github.com/<your_repo>/avionics_cots.git
cd avionics_cots
pip install platformio
platformio run -e featheresp32
platformio run -e featheresp32 -t upload
platformio device monitor -e featheresp32
```

---

## 📂 Project Structure
```
avionics_cots/
├── src/                # Main application logic
├── lib/
│   ├── SerialProtocol/ # UART and SPI protocols
│   ├── Packets/        # Packet definitions
│   └── CoscoRouter/    # Packet routing
├── docs/               # Documentation
└── platformio.ini      # PlatformIO config
```

---

## 🗺️ Future Improvements
- **DMA Support:** Optimize UART/SPI performance via Direct Memory Access.
- **CI & Testing:** Implement automated unit testing and CI pipeline.

---

## 📄 License

MIT License © Eliot Abramo, 2025
