#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include <NeoPixelBus.h>
#include <ArduinoOTA.h>
#include "config.h"
#include "effects.h"
#include "web_server.h"

// LED strip - NeoPixelBus with ESP32 RMT
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod> strip(NUM_LEDS, LED_PIN);

// Global objects
LEDEffects* ledEffects;
LEDWebServer* webServer;

// WiFi mode flag
bool isAPMode = false;

// Prototypes
void setupWiFi();
void setupAP();
void setupOTA();
void loadSettings();
void saveSettings();
void printStartupInfo();

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("       ⭐ LED Star Controller ⭐");
    Serial.println("========================================\n");
    
    // Init EEPROM
    EEPROM.begin(EEPROM_SIZE);
    
    // Init NeoPixelBus
    Serial.print("Инициализация LED ленты... ");
    strip.Begin();
    strip.Show(); // Safe on NeoPixelBus - proper RMT init
    Serial.println("OK");
    
    // Create effects manager
    ledEffects = new LEDEffects(&strip, NUM_LEDS);
    
    // Load saved settings
    loadSettings();
    
    // Connect to WiFi
    setupWiFi();
    
    // Setup OTA
    setupOTA();

    // Start web server
    webServer = new LEDWebServer(ledEffects, WEB_SERVER_PORT);
    webServer->setOnSettingsChange(saveSettings);
    webServer->begin();
    
    // Show startup color
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.SetPixelColor(i, isAPMode ? RgbColor(255, 128, 0) : RgbColor(0, 255, 0));
    }
    strip.Show();
    delay(500);
    for (int i = 0; i < NUM_LEDS; i++) {
        strip.SetPixelColor(i, RgbColor(0, 0, 0));
    }
    strip.Show();
    
    printStartupInfo();
}

void loop() {
    // OTA handling
    ArduinoOTA.handle();
    
    // Web server - needed for HTTP OTA
    webServer->handleClient();
    
    // Update effects
    ledEffects->update();
    
    yield();
}

void setupWiFi() {
    Serial.print("Подключение к WiFi: ");
    Serial.println(WIFI_SSID);
    
    #if STATIC_IP_ENABLED
    IPAddress staticIP(STATIC_IP);
    IPAddress gateway(GATEWAY_IP);
    IPAddress subnet(SUBNET_MASK);
    IPAddress dns(DNS_IP);
    WiFi.config(staticIP, gateway, subnet, dns);
    #endif
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < WIFI_CONNECT_TIMEOUT * 2) {
        Serial.print(".");
        attempts++;
        delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        isAPMode = false;
        Serial.println("\n✓ WiFi подключен!");
        Serial.print("  IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n✗ WiFi не доступен");
        setupAP();
    }
}

void setupAP() {
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    
    IPAddress apIP(AP_IP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASSWORD, AP_CHANNEL, false, AP_MAX_CONNECTIONS);
    
    isAPMode = true;
    
    Serial.println("✓ Точка доступа: " + String(AP_SSID));
    Serial.print("  IP: ");
    Serial.println(WiFi.softAPIP());
}

void setupOTA() {
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    
    ArduinoOTA.onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH) {
                type = "sketch";
            } else {
                type = "filesystem";
            }
            String msg = "Start " + type;
            Serial.println("OTA " + msg);
            if (webServer) {
                webServer->setOtaState(msg);
                webServer->setOtaProgress(0);
                webServer->setOtaError(""); // Clear previous error
            }
        })
        .onEnd([]() {
            Serial.println("\nOTA End");
            if (webServer) {
                webServer->setOtaState("Success");
                webServer->setOtaProgress(100);
            }
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            unsigned int p = (progress / (total / 100));
            Serial.printf("OTA Progress: %u%%\r", p);
            if (webServer) webServer->setOtaProgress(p);
        })
        .onError([](ota_error_t error) {
            String errorMsg = "Error[" + String(error) + "]: ";
            if (error == OTA_AUTH_ERROR) errorMsg += "Auth Failed";
            else if (error == OTA_BEGIN_ERROR) errorMsg += "Begin Failed";
            else if (error == OTA_CONNECT_ERROR) errorMsg += "Connect Failed";
            else if (error == OTA_RECEIVE_ERROR) errorMsg += "Receive Failed";
            else if (error == OTA_END_ERROR) errorMsg += "End Failed";
            else errorMsg += "Unknown";
            
            Serial.println("OTA " + errorMsg);
            if (webServer) {
                webServer->setOtaError(errorMsg);
                webServer->setOtaState("Failed");
            }
        });
    
    ArduinoOTA.begin();
    Serial.println("✓ OTA готов на порту 3232");
}

void loadSettings() {
    Serial.print("Загрузка настроек... ");
    
    if (EEPROM.read(EEPROM_MAGIC_ADDR) == EEPROM_MAGIC_VALUE) {
        uint8_t effect = EEPROM.read(EEPROM_EFFECT_ADDR);
        uint8_t brightness = EEPROM.read(EEPROM_BRIGHTNESS_ADDR);
        uint8_t speed = EEPROM.read(EEPROM_SPEED_ADDR);
        uint8_t r = EEPROM.read(EEPROM_COLOR_R_ADDR);
        uint8_t g = EEPROM.read(EEPROM_COLOR_G_ADDR);
        uint8_t b = EEPROM.read(EEPROM_COLOR_B_ADDR);
        
        if (effect < EFFECT_COUNT) {
            ledEffects->setEffect((Effect)effect);
        }
        ledEffects->setBrightness(brightness);
        ledEffects->setSpeed(speed);
        ledEffects->setColor(r, g, b);
        
        // Load auto-cycle settings
        bool autoCycle = EEPROM.read(EEPROM_AUTOCYCLE_ADDR) == 1;
        uint16_t cycleTime = (EEPROM.read(EEPROM_CYCLETIME_HI_ADDR) << 8) | EEPROM.read(EEPROM_CYCLETIME_LO_ADDR);
        if (cycleTime == 0 || cycleTime > 300) cycleTime = 30;
        ledEffects->setAutoCycle(autoCycle);
        ledEffects->setCycleTime(cycleTime);
        
        // Load palette
        uint8_t palSize = EEPROM.read(EEPROM_PALETTE_SIZE_ADDR);
        if (palSize >= 1 && palSize <= MAX_PALETTE_COLORS) {
            ledEffects->setPaletteSize(palSize);
            for (int i = 0; i < palSize; i++) {
                uint8_t pr = EEPROM.read(EEPROM_PALETTE_START_ADDR + i * 3);
                uint8_t pg = EEPROM.read(EEPROM_PALETTE_START_ADDR + i * 3 + 1);
                uint8_t pb = EEPROM.read(EEPROM_PALETTE_START_ADDR + i * 3 + 2);
                ledEffects->setPaletteColor(i, pr, pg, pb);
            }
        }
        
        Serial.println("OK");
    } else {
        Serial.println("(defaults)");
    }
}

void saveSettings() {
    static unsigned long lastSave = 0;
    if (millis() - lastSave < 5000) return;
    lastSave = millis();
    
    uint8_t r, g, b;
    ledEffects->getColor(&r, &g, &b);
    
    EEPROM.write(EEPROM_EFFECT_ADDR, ledEffects->getEffect());
    EEPROM.write(EEPROM_BRIGHTNESS_ADDR, ledEffects->getBrightness());
    EEPROM.write(EEPROM_SPEED_ADDR, ledEffects->getSpeed());
    EEPROM.write(EEPROM_COLOR_R_ADDR, r);
    EEPROM.write(EEPROM_COLOR_G_ADDR, g);
    EEPROM.write(EEPROM_COLOR_B_ADDR, b);
    
    // Save auto-cycle settings
    EEPROM.write(EEPROM_AUTOCYCLE_ADDR, ledEffects->getAutoCycle() ? 1 : 0);
    uint16_t cycleTime = ledEffects->getCycleTime();
    EEPROM.write(EEPROM_CYCLETIME_HI_ADDR, cycleTime >> 8);
    EEPROM.write(EEPROM_CYCLETIME_LO_ADDR, cycleTime & 0xFF);
    
    // Save palette
    uint8_t palSize = ledEffects->getPaletteSize();
    EEPROM.write(EEPROM_PALETTE_SIZE_ADDR, palSize);
    for (int i = 0; i < palSize; i++) {
        uint8_t pr, pg, pb;
        ledEffects->getPaletteColor(i, &pr, &pg, &pb);
        EEPROM.write(EEPROM_PALETTE_START_ADDR + i * 3, pr);
        EEPROM.write(EEPROM_PALETTE_START_ADDR + i * 3 + 1, pg);
        EEPROM.write(EEPROM_PALETTE_START_ADDR + i * 3 + 2, pb);
    }
    
    EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
    
    EEPROM.commit();
    Serial.println("⚡ Saved");
}

void printStartupInfo() {
    Serial.println("\n========================================");
    Serial.println("         Готово!");
    Serial.println("========================================");
    Serial.print("  http://");
    Serial.println(isAPMode ? WiFi.softAPIP() : WiFi.localIP());
    Serial.println("========================================\n");
}
