#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include "effects.h"

class LEDWebServer {
public:
    LEDWebServer(LEDEffects* effects, uint16_t port = 80);
    void begin();
    void handleClient();
    void setOnSettingsChange(void (*callback)());
    void setOtaError(String error) { otaError = error; }
    void setOtaProgress(uint8_t progress) { otaProgress = progress; }
    void setOtaState(String state) { otaState = state; }

private:
    WebServer server;
    LEDEffects* effects;
    void (*onSettingsChange)();
    String otaError;
    String otaState = "Idle";
    uint8_t otaProgress = 0;

    void handleRoot();
    void handleGetStatus();
    void handleSetEffect();
    void handleSetBrightness();
    void handleSetSpeed();
    void handleSetColor();
    void handleSetAutoCycle();
    void handleSetPalette();
    void handleReboot();
    void handleDebug();
    void handleNotFound();
    void handleUpdateUpload();
    void handleUpdateEnd();
    String generateHTML();
};

#endif
