<div align="center">

# star

**WS2812B LED strip controller with 23 effects and OTA updates for ESP32-C3**

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

<div align="center">

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

</div>

## ■ How It Works

```
1. On boot, ESP32-C3 connects to the configured WiFi network; if unavailable, it starts an AP (LED-Star) for standalone access.
2. The built-in HTTP WebServer serves a self-contained page at the static IP (192.168.1.252) with color picker, brightness/speed sliders, and effect groups.
3. The selected effect is rendered on the WS2812B strip via NeoPixelBus using the RMT peripheral.
4. All settings (effect, brightness, speed, color, palette, auto-cycle interval) are persisted to EEPROM and restored on the next boot.
5. Firmware can be updated over WiFi via ArduinoOTA (espota, port 3232) or HTTP file upload to /api/update.
```

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

## ■ Wiring

<div align="center">

| ESP32-C3 | WS2812B |
|----------|---------|
| GPIO 3 | DIN |
| 5V | VCC |
| GND | GND |

</div>

## ■ License

MIT © [pluttan](https://github.com/pluttan)
