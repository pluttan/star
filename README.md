![Header](header.png)

<div align="center">

# star

**WS2812B LED strip controller with 23 effects and OTA updates for ESP32-C3**

[![License](https://img.shields.io/badge/license-MIT-2C2C2C?style=for-the-badge&labelColor=1E1E1E)](LICENSE)
[![ESP32-C3](https://img.shields.io/badge/ESP32--C3-RISC--V-2C2C2C?style=for-the-badge&logo=espressif&labelColor=1E1E1E)]()
[![NeoPixelBus](https://img.shields.io/badge/NeoPixelBus-2.7-2C2C2C?style=for-the-badge&labelColor=1E1E1E)]()
[![PlatformIO](https://img.shields.io/badge/PlatformIO-build-2C2C2C?style=for-the-badge&logo=platformio&labelColor=1E1E1E)]()

</div>

WS2812B addressable LED strip controller running on ESP32-C3 with a self-contained web interface. Ships with 23 built-in effects grouped into Solid / Rainbow / White / Nature / Holiday, persistent settings in EEPROM, dual OTA firmware updates (ArduinoOTA + HTTP), and AP fallback mode for standalone operation.

## ■ Features

- ❖ **23 effects** — grouped into Solid, Rainbow, White, Nature, and Holiday (Static, Breathing, Pulse, Fire, Meteor, Ocean, Lava, Forest, Stars, Christmas, Police, Running Lights, and more)
- ❖ **Web interface** — single self-contained responsive page with color picker, brightness/speed sliders, and effect groups
- ❖ **OTA updates** — reflash over WiFi via ArduinoOTA (espota, port 3232) or HTTP upload to `/api/update`
- ❖ **Auto-cycle** — automatically rotate through effects on a configurable interval
- ❖ **Custom palette** — up to 7 blended colors for palette-driven effects
- ❖ **Persistent settings** — effect, brightness, speed, color, palette, and auto-cycle saved to EEPROM across reboots
- ❖ **AP fallback** — creates its own WiFi network (`LED-Star`) if the home network is unavailable
- ❖ **Static IP** — fixed local network address (`192.168.1.252`) for reliable access
- ❖ **Color presets** — 8 quick-select swatches plus an arbitrary hex color picker

## ■ Stack

| Component | Technology |
|-----------|------------|
| MCU | ESP32-C3 (RISC-V) |
| LED | WS2812B (RMT-driven, configurable count) |
| Framework | Arduino |
| LED library | NeoPixelBus 2.7 |
| JSON | ArduinoJson 7.0 |
| OTA | ArduinoOTA + HTTP Update |
| Build | PlatformIO |
| Interface | Built-in synchronous WebServer (HTTP) |

## ■ Wiring

| ESP32-C3 | WS2812B |
|----------|---------|
| GPIO 3 | DIN |
| 5V | VCC |
| GND | GND |

## ■ Usage

```bash
# Build, upload via USB, and open serial monitor
make flash

# Build + USB upload only
make upload

# Serial monitor
make monitor

# OTA update over WiFi (espota to 192.168.1.252)
make ota

# HTTP OTA upload of a built firmware image
python3 ota_upload.py .pio/build/esp32c3/firmware.bin [ip]
```

Configuration is in `include/config.h` — WiFi credentials, static IP, AP fallback, OTA hostname, EEPROM layout. LED pin (GPIO 3) and LED count are set via `build_flags` in `platformio.ini`.

## ■ License

MIT © [pluttan](https://github.com/pluttan)
