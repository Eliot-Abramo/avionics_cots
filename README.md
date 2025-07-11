# Avionics COTS – SPI Bus Edition

**Status:** Prototype ‑ Feature branch `spi_bus`  
**Platform:** Feather ESP32 (Arduino framework, PlatformIO)  
**Primary Language:** C++17

## ✈️ Overview
This project is part of an **avionics control stack** designed for low‑cost autonomous vehicles (rovers / drones / rocketry).  
The *SPI Bus* feature branch refactors the I2C‑centric architecture to a **bi‑directional SPI slave bus** for higher bandwidth and noise immunity.

<img src="docs/architecture.svg" width="600" alt="Subsystem Architecture"/>

## 🛠️ Main Features
| Subsystem | Driver / File | IC / Sensor | Purpose |
|-----------|---------------|-------------|---------|
| **Dust sensing** | `lib/Dust_Driver/*` | HM3301 | PM2.5 / PM10 air quality |
| **Mass measurement** | HX711 (`GyverHX711`) | Load cells | Drill mass & rover mass |
| **Servo control** | `lib/Servo_Driver/*` / `ESP32Servo` | Generic PWM | Cam & drill hatch |
| **LED feedback** | NeoPixel & FastLED | WS2812B | System state & debugging |
| **SPI Slave comms** | `ESP32SPISlave` | ESP32 VSPI | High‑speed packet interface |
| **Packet layer** | `lib/Packets/*` | Custom structs | Typed messages with CRC8 |
| **Monitor utils** | `lib/Monitor/*` | Serial | Benchmark & debug |

All subsystems are orchestrated from `src/main.cpp`, which runs a 100 Hz loop and pushes telemetry packets via SPI every 50 ms.

## 🔧 Build & Flash

```bash
# 1. Install PlatformIO (VSCode extension or CLI)
git clone https://github.com/<your‑org>/avionics_cots.git
cd avionics_cots
pio run                 # build
pio device monitor      # serial monitor
pio run -t upload       # flash over /dev/ttyBMS
```

*`platformio.ini`* pins the environment to `espressif32@5.4.0` to avoid breaking changes.

## 📡 Packet Protocol
* Located in `lib/Packets/packet_definition.hpp`
* 1‑byte **ID**, 1‑byte **length**, <payload>, **CRC‑8/MAXIM**
* Endianness: Little‑endian
* Helper `Cosco` class provides TX/RX helpers and a **callback registry** for each packet type.

## 🚀 Roadmap
- [ ] Replace blocking HX711 reads with DMA‑driven averaging
- [ ] Migrate dust sensor to external I2C expander
- [ ] Add unit tests via Unity + fake‑Arduino
- [ ] Continuous integration (GitHub Actions)

## 🤝 Contributing
PRs are welcome! Please run `clang-format` and `cppcheck` before submitting.

## 📄 License
MIT
