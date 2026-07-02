![Header](header.png)

<div align="center">

# star

**Контроллер светодиодной ленты WS2812B с 23 эффектами и OTA-обновлениями для ESP32-C3**

[![License](https://img.shields.io/badge/license-MIT-2C2C2C?style=for-the-badge&labelColor=1E1E1E)](LICENSE)
[![ESP32-C3](https://img.shields.io/badge/ESP32--C3-RISC--V-2C2C2C?style=for-the-badge&logo=espressif&labelColor=1E1E1E)]()
[![NeoPixelBus](https://img.shields.io/badge/NeoPixelBus-2.7-2C2C2C?style=for-the-badge&labelColor=1E1E1E)]()
[![PlatformIO](https://img.shields.io/badge/PlatformIO-build-2C2C2C?style=for-the-badge&logo=platformio&labelColor=1E1E1E)]()

</div>

Контроллер адресуемой светодиодной ленты WS2812B на базе ESP32-C3 со встроенным веб-интерфейсом. В комплекте 23 встроенных эффекта, сгруппированных в категории Solid / Rainbow / White / Nature / Holiday, постоянное хранение настроек в EEPROM, двойное OTA-обновление прошивки (ArduinoOTA + HTTP) и режим точки доступа для автономной работы.

## ■ Возможности

- ❖ **23 эффекта** — сгруппированы в Solid, Rainbow, White, Nature и Holiday (Static, Breathing, Pulse, Fire, Meteor, Ocean, Lava, Forest, Stars, Christmas, Police, Running Lights и другие)
- ❖ **Веб-интерфейс** — единая самодостаточная адаптивная страница с палитрой цветов, ползунками яркости/скорости и группами эффектов
- ❖ **OTA-обновления** — перепрошивка по WiFi через ArduinoOTA (espota, порт 3232) или HTTP-загрузка на `/api/update`
- ❖ **Автосмена** — автоматическая ротация эффектов с настраиваемым интервалом
- ❖ **Пользовательская палитра** — до 7 смешиваемых цветов для эффектов на основе палитры
- ❖ **Постоянные настройки** — эффект, яркость, скорость, цвет, палитра и автосмена сохраняются в EEPROM между перезагрузками
- ❖ **Резервная точка доступа** — создаёт собственную WiFi-сеть (`LED-Star`), если домашняя сеть недоступна
- ❖ **Статический IP** — фиксированный адрес в локальной сети (`192.168.1.252`) для надёжного доступа
- ❖ **Предустановки цветов** — 8 быстро выбираемых образцов плюс произвольный выбор цвета по hex-коду

## ■ Стек

<div align="center">

| Компонент | Technology |
|-----------|------------|
| МК | ESP32-C3 (RISC-V) |
| Лента | WS2812B (RMT-driven, configurable count) |
| Фреймворк | Arduino |
| Библиотека LED | NeoPixelBus 2.7 |
| JSON | ArduinoJson 7.0 |
| OTA | ArduinoOTA + HTTP Update |
| Сборка | PlatformIO |
| Интерфейс | Built-in synchronous WebServer (HTTP) |

</div>

## ■ Подключение

<div align="center">

| ESP32-C3 | WS2812B |
|----------|---------|
| GPIO 3 | DIN |
| 5V | VCC |
| GND | GND |

</div>

## ■ Запуск

```bash
# Сборка, загрузка через USB и открытие монитора порта
make flash

# Только сборка + загрузка через USB
make upload

# Монитор порта
make monitor

# OTA-обновление по WiFi (espota на 192.168.1.252)
make ota

# HTTP OTA загрузка собранного образа прошивки
python3 ota_upload.py .pio/build/esp32c3/firmware.bin [ip]
```

Конфигурация находится в `include/config.h` — учётные данные WiFi, статический IP, резервная точка доступа, OTA-имя хоста, разметка EEPROM. Пин LED (GPIO 3) и количество светодиодов задаются через `build_flags` в `platformio.ini`.

## ■ License

MIT © [pluttan](https://github.com/pluttan)
