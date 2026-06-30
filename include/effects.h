#ifndef EFFECTS_H
#define EFFECTS_H

#include <NeoPixelBus.h>
#include "config.h"

typedef RgbColor Color;

#define MAX_PALETTE_COLORS 7

// Группы эффектов для UI
enum EffectGroup {
    GROUP_SOLID = 0,
    GROUP_RAINBOW,
    GROUP_WHITE,
    GROUP_NATURE,
    GROUP_HOLIDAY,
    GROUP_COUNT
};

const char* const GROUP_NAMES[] = {
    "Solid",
    "Rainbow", 
    "White",
    "Nature",
    "Holiday"
};

// Эффекты - убраны дёрганные (strobe, sparkle, theater)
enum Effect {
    // Solid
    EFFECT_STATIC = 0,
    EFFECT_BREATHING,
    EFFECT_PULSE,
    EFFECT_FADE_IN_OUT,
    // Rainbow
    EFFECT_RAINBOW,
    EFFECT_RAINBOW_WAVE,
    EFFECT_RAINBOW_CYCLE,
    EFFECT_RAINBOW_COMET,
    EFFECT_GRADIENT,
    // White
    EFFECT_WHITE_BREATHING,
    EFFECT_WHITE_WAVE,
    EFFECT_WHITE_TWINKLE,
    EFFECT_SNOW,
    EFFECT_CANDLE,
    // Nature
    EFFECT_FIRE,
    EFFECT_METEOR,
    EFFECT_OCEAN,
    EFFECT_LAVA,
    EFFECT_FOREST,
    EFFECT_STARS,
    // Holiday
    EFFECT_CHRISTMAS,
    EFFECT_POLICE,
    EFFECT_RUNNING_LIGHTS,
    EFFECT_COUNT
};

const char* const EFFECT_NAMES[] = {
    // Solid
    "Static", "Breathing", "Pulse", "Color Fade",
    // Rainbow
    "Rainbow", "Rainbow Wave", "Rainbow Cycle", "Rainbow Comet", "Gradient",
    // White
    "White Breath", "White Wave", "White Twinkle", "Snow", "Candle",
    // Nature
    "Fire", "Meteor", "Ocean", "Lava", "Forest", "Stars",
    // Holiday
    "Christmas", "Police", "Running Lights"
};

// Группа для каждого эффекта
const EffectGroup EFFECT_GROUPS[] = {
    GROUP_SOLID, GROUP_SOLID, GROUP_SOLID, GROUP_SOLID,
    GROUP_RAINBOW, GROUP_RAINBOW, GROUP_RAINBOW, GROUP_RAINBOW, GROUP_RAINBOW,
    GROUP_WHITE, GROUP_WHITE, GROUP_WHITE, GROUP_WHITE, GROUP_WHITE,
    GROUP_NATURE, GROUP_NATURE, GROUP_NATURE, GROUP_NATURE, GROUP_NATURE, GROUP_NATURE,
    GROUP_HOLIDAY, GROUP_HOLIDAY, GROUP_HOLIDAY
};

class LEDEffects {
public:
    LEDEffects(NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod>* strip, uint16_t numLeds);

    void setEffect(Effect effect);
    Effect getEffect() const { return currentEffect; }
    
    void setSpeed(uint8_t speed);
    uint8_t getSpeed() const { return effectSpeed; }
    
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness() const { return currentBrightness; }
    
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void getColor(uint8_t* r, uint8_t* g, uint8_t* b) const;
    
    // Auto-cycle
    void setAutoCycle(bool enabled);
    bool getAutoCycle() const { return autoCycleEnabled; }
    void setCycleTime(uint16_t seconds);
    uint16_t getCycleTime() const { return cycleTime; }
    
    // Palette
    void setPaletteColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
    void getPaletteColor(uint8_t index, uint8_t* r, uint8_t* g, uint8_t* b) const;
    void setPaletteSize(uint8_t size);
    uint8_t getPaletteSize() const { return paletteSize; }
    
    void update();

private:
    NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod>* strip;
    uint16_t numLeds;
    Effect currentEffect;
    uint8_t effectSpeed;
    uint8_t currentBrightness;
    uint8_t colorR, colorG, colorB;
    
    // Auto-cycle
    bool autoCycleEnabled;
    uint16_t cycleTime; // seconds
    unsigned long lastCycleChange;
    
    unsigned long lastUpdate;
    uint16_t step;
    uint8_t hue;
    
    // Palette
    Color palette[MAX_PALETTE_COLORS];
    uint8_t paletteSize;

    Color applyBrightness(Color color);
    Color applyBrightness(Color color, uint8_t brightness);
    Color hsvToRgb(uint8_t h, uint8_t s, uint8_t v);
    void fillSolid(Color color);
    void clearAll();
    void fadeAll(uint8_t amount);
    void nextEffect();
    Color blendPalette(float position); // Blend through palette colors

    // Effects
    void effectStatic();
    void effectBreathing();
    void effectPulse();
    void effectFadeInOut();
    void effectRainbow();
    void effectRainbowWave();
    void effectRainbowCycle();
    void effectRainbowComet();
    void effectGradient();
    void effectWhiteBreathing();
    void effectWhiteWave();
    void effectWhiteTwinkle();
    void effectSnow();
    void effectCandle();
    void effectFire();
    void effectMeteor();
    void effectOcean();
    void effectLava();
    void effectForest();
    void effectStars();
    void effectChristmas();
    void effectPolice();
    void effectRunningLights();
};

#endif
