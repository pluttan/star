#include "effects.h"

LEDEffects::LEDEffects(NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Ws2812xMethod>* strip, uint16_t numLeds)
    : strip(strip), numLeds(numLeds), currentEffect(EFFECT_RAINBOW),
      effectSpeed(EFFECT_SPEED_DEFAULT), currentBrightness(DEFAULT_BRIGHTNESS),
      colorR(255), colorG(0), colorB(0),
      autoCycleEnabled(false), cycleTime(30), lastCycleChange(0),
      lastUpdate(0), step(0), hue(0), paletteSize(2) {
    // Default palette: rainbow-like
    palette[0] = Color(255, 0, 0);    // Red
    palette[1] = Color(255, 165, 0);  // Orange
    palette[2] = Color(255, 255, 0);  // Yellow
    palette[3] = Color(0, 255, 0);    // Green
    palette[4] = Color(0, 255, 255);  // Cyan
    palette[5] = Color(0, 0, 255);    // Blue
    palette[6] = Color(255, 0, 255);  // Magenta
}

void LEDEffects::setEffect(Effect effect) {
    if (effect < EFFECT_COUNT) {
        currentEffect = effect;
        step = 0;
        hue = 0;
        lastCycleChange = millis();
        clearAll();
    }
}

void LEDEffects::setSpeed(uint8_t speed) {
    effectSpeed = constrain(speed, EFFECT_SPEED_MIN, EFFECT_SPEED_MAX);
}

void LEDEffects::setBrightness(uint8_t brightness) {
    currentBrightness = brightness;
}

void LEDEffects::setColor(uint8_t r, uint8_t g, uint8_t b) {
    colorR = r; colorG = g; colorB = b;
}

void LEDEffects::getColor(uint8_t* r, uint8_t* g, uint8_t* b) const {
    *r = colorR; *g = colorG; *b = colorB;
}

void LEDEffects::setAutoCycle(bool enabled) {
    autoCycleEnabled = enabled;
    lastCycleChange = millis();
}

void LEDEffects::setCycleTime(uint16_t seconds) {
    cycleTime = seconds > 0 ? seconds : 1;
}

void LEDEffects::setPaletteColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < MAX_PALETTE_COLORS) {
        palette[index] = Color(r, g, b);
    }
}

void LEDEffects::getPaletteColor(uint8_t index, uint8_t* r, uint8_t* g, uint8_t* b) const {
    if (index < MAX_PALETTE_COLORS) {
        *r = palette[index].R;
        *g = palette[index].G;
        *b = palette[index].B;
    }
}

void LEDEffects::setPaletteSize(uint8_t size) {
    paletteSize = constrain(size, 1, MAX_PALETTE_COLORS);
}

void LEDEffects::nextEffect() {
    Effect next = (Effect)((currentEffect + 1) % EFFECT_COUNT);
    setEffect(next);
}

Color LEDEffects::applyBrightness(Color color) {
    return applyBrightness(color, currentBrightness);
}

Color LEDEffects::applyBrightness(Color color, uint8_t brightness) {
    return Color((color.R * brightness) / 255, (color.G * brightness) / 255, (color.B * brightness) / 255);
}

Color LEDEffects::hsvToRgb(uint8_t h, uint8_t s, uint8_t v) {
    if (s == 0) return Color(v, v, v);
    uint8_t region = h / 43;
    uint8_t remainder = (h - (region * 43)) * 6;
    uint8_t p = (v * (255 - s)) >> 8;
    uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    switch (region) {
        case 0:  return Color(v, t, p);
        case 1:  return Color(q, v, p);
        case 2:  return Color(p, v, t);
        case 3:  return Color(p, q, v);
        case 4:  return Color(t, p, v);
        default: return Color(v, p, q);
    }
}

Color LEDEffects::blendPalette(float position) {
    if (paletteSize == 1) return palette[0];
    
    // Clamp and scale position to palette range
    position = constrain(position, 0.0f, 1.0f);
    float scaled = position * (paletteSize - 1);
    uint8_t idx1 = (uint8_t)scaled;
    uint8_t idx2 = min(idx1 + 1, paletteSize - 1);
    float blend = scaled - idx1;
    
    // Linear interpolation between two adjacent colors
    return Color(
        palette[idx1].R + (palette[idx2].R - palette[idx1].R) * blend,
        palette[idx1].G + (palette[idx2].G - palette[idx1].G) * blend,
        palette[idx1].B + (palette[idx2].B - palette[idx1].B) * blend
    );
}

void LEDEffects::fillSolid(Color color) {
    for (uint16_t i = 0; i < numLeds; i++) strip->SetPixelColor(i, color);
}

void LEDEffects::clearAll() { fillSolid(Color(0, 0, 0)); }

void LEDEffects::fadeAll(uint8_t amount) {
    for (uint16_t i = 0; i < numLeds; i++) {
        Color c = strip->GetPixelColor(i);
        strip->SetPixelColor(i, Color(
            c.R > amount ? c.R - amount : 0,
            c.G > amount ? c.G - amount : 0,
            c.B > amount ? c.B - amount : 0));
    }
}

void LEDEffects::update() {
    unsigned long now = millis();
    
    // Auto-cycle
    if (autoCycleEnabled && (now - lastCycleChange >= cycleTime * 1000UL)) {
        nextEffect();
    }
    
    // Effect update rate
    unsigned long interval = map(effectSpeed, 1, 255, 80, 8);
    if (now - lastUpdate < interval) return;
    lastUpdate = now;

    switch (currentEffect) {
        case EFFECT_STATIC: effectStatic(); break;
        case EFFECT_BREATHING: effectBreathing(); break;
        case EFFECT_PULSE: effectPulse(); break;
        case EFFECT_FADE_IN_OUT: effectFadeInOut(); break;
        case EFFECT_RAINBOW: effectRainbow(); break;
        case EFFECT_RAINBOW_WAVE: effectRainbowWave(); break;
        case EFFECT_RAINBOW_CYCLE: effectRainbowCycle(); break;
        case EFFECT_RAINBOW_COMET: effectRainbowComet(); break;
        case EFFECT_GRADIENT: effectGradient(); break;
        case EFFECT_WHITE_BREATHING: effectWhiteBreathing(); break;
        case EFFECT_WHITE_WAVE: effectWhiteWave(); break;
        case EFFECT_WHITE_TWINKLE: effectWhiteTwinkle(); break;
        case EFFECT_SNOW: effectSnow(); break;
        case EFFECT_CANDLE: effectCandle(); break;
        case EFFECT_FIRE: effectFire(); break;
        case EFFECT_METEOR: effectMeteor(); break;
        case EFFECT_OCEAN: effectOcean(); break;
        case EFFECT_LAVA: effectLava(); break;
        case EFFECT_FOREST: effectForest(); break;
        case EFFECT_STARS: effectStars(); break;
        case EFFECT_CHRISTMAS: effectChristmas(); break;
        case EFFECT_POLICE: effectPolice(); break;
        case EFFECT_RUNNING_LIGHTS: effectRunningLights(); break;
        default: effectRainbow(); break;
    }
    strip->Show();
}

// === SOLID GROUP ===
void LEDEffects::effectStatic() {
    fillSolid(applyBrightness(palette[0]));
}

void LEDEffects::effectBreathing() {
    float breath = (sin(millis() / 2500.0 * PI) + 1) / 2;
    uint8_t b = breath * currentBrightness;
    Color c = palette[0];
    fillSolid(Color(c.R * b / 255, c.G * b / 255, c.B * b / 255));
}

void LEDEffects::effectPulse() {
    uint8_t center = numLeds / 2;
    float wave = (sin(step * 0.15) + 1) / 2;
    uint8_t dist = wave * center;
    clearAll();
    Color c1 = palette[0];
    Color c2 = paletteSize > 1 ? palette[1] : palette[0];
    for (int i = -1; i <= 1; i++) {
        if (center - dist + i >= 0 && center - dist + i < numLeds)
            strip->SetPixelColor(center - dist + i, applyBrightness(c1));
        if (center + dist + i >= 0 && center + dist + i < numLeds)
            strip->SetPixelColor(center + dist + i, applyBrightness(c2));
    }
    step++;
}

void LEDEffects::effectFadeInOut() {
    uint8_t brightness = (sin(step * 0.03) + 1) / 2 * currentBrightness;
    // Fade through palette colors
    float palettePos = (step % 500) / 500.0;
    fillSolid(applyBrightness(blendPalette(palettePos), brightness));
    step++;
}

// === RAINBOW GROUP ===
void LEDEffects::effectRainbow() {
    for (uint16_t i = 0; i < numLeds; i++) {
        // Blend through palette across strip, animate over time
        float pos = fmod((float)i / numLeds + step * 0.005, 1.0);
        strip->SetPixelColor(i, applyBrightness(blendPalette(pos)));
    }
    step++;
}

void LEDEffects::effectRainbowWave() {
    for (uint16_t i = 0; i < numLeds; i++) {
        // Faster wave effect through palette
        float pos = fmod((float)i / numLeds * 3 + step * 0.02, 1.0);
        strip->SetPixelColor(i, applyBrightness(blendPalette(pos)));
    }
    step++;
}

void LEDEffects::effectRainbowCycle() {
    for (uint16_t i = 0; i < numLeds; i++) {
        // Full palette cycle across strip
        float pos = fmod((float)i / numLeds + step * 0.008, 1.0);
        strip->SetPixelColor(i, applyBrightness(blendPalette(pos)));
    }
    step++;
}

void LEDEffects::effectRainbowComet() {
    fadeAll(25);
    uint16_t pos = step % (numLeds + 8);
    for (int i = 0; i < 6; i++) {
        int p = pos - i;
        if (p >= 0 && p < numLeds) {
            uint8_t b = 255 - i * 40;
            // Comet color from palette position
            float palettePos = fmod(step * 0.02, 1.0);
            strip->SetPixelColor(p, applyBrightness(blendPalette(palettePos), b));
        }
    }
    step++;
}

void LEDEffects::effectGradient() {
    for (uint16_t i = 0; i < numLeds; i++) {
        // Smooth gradient across strip using palette
        float pos = fmod((float)i / numLeds + step * 0.003, 1.0);
        strip->SetPixelColor(i, applyBrightness(blendPalette(pos)));
    }
    step++;
}

// === WHITE GROUP ===
void LEDEffects::effectWhiteBreathing() {
    float breath = (sin(millis() / 2500.0 * PI) + 1) / 2;
    uint8_t b = breath * currentBrightness;
    // Use palette[0] for breathing (default: warm white)
    Color c = palette[0];
    fillSolid(Color(c.R * b / 255, c.G * b / 255, c.B * b / 255));
}

void LEDEffects::effectWhiteWave() {
    for (uint16_t i = 0; i < numLeds; i++) {
        float wave = (sin((i + step) * 0.25) + 1) / 2;
        uint8_t b = wave * currentBrightness;
        // Blend between palette[0] and palette[1]
        Color c = blendPalette((float)i / numLeds);
        strip->SetPixelColor(i, Color(c.R * b / 255, c.G * b / 255, c.B * b / 255));
    }
    step++;
}

void LEDEffects::effectWhiteTwinkle() {
    fadeAll(8);
    if (random(100) < 25) {
        int pos = random(numLeds);
        uint8_t bright = random(180, 255);
        // Pick random color from palette
        uint8_t idx = random(paletteSize);
        strip->SetPixelColor(pos, applyBrightness(palette[idx], bright));
    }
}

void LEDEffects::effectSnow() {
    fadeAll(4);
    if (random(100) < 20) {
        // Use palette[0] for snow flakes
        strip->SetPixelColor(random(numLeds), applyBrightness(palette[0]));
    }
}

void LEDEffects::effectCandle() {
    for (uint16_t i = 0; i < numLeds; i++) {
        // Use palette colors with random flicker variation
        uint8_t flicker = random(180, 255);
        float pos = random(100) / 100.0;
        Color c = blendPalette(pos);
        strip->SetPixelColor(i, applyBrightness(c, flicker));
    }
}

// === NATURE GROUP ===
void LEDEffects::effectFire() {
    for (uint16_t i = 0; i < numLeds; i++) {
        // Blend between palette colors based on heat
        float heat = random(50, 100) / 100.0;
        Color c = blendPalette(heat);
        uint8_t flicker = random(180, 255);
        strip->SetPixelColor(i, applyBrightness(c, flicker));
    }
}

void LEDEffects::effectMeteor() {
    fadeAll(35);
    for (int j = 0; j < 6; j++) {
        int pos = step - j;
        if (pos >= 0 && pos < numLeds) {
            uint8_t fade = 255 - j * 40;
            // Blend through palette for meteor trail
            float palettePos = (float)j / 5.0;
            Color c = blendPalette(palettePos);
            strip->SetPixelColor(pos, applyBrightness(c, fade));
        }
    }
    step++;
    if (step >= numLeds + 12) step = 0;
}

void LEDEffects::effectOcean() {
    for (uint16_t i = 0; i < numLeds; i++) {
        float wave = (sin((i * 0.2) + step * 0.04) + sin((i * 0.35) + step * 0.025)) / 2 + 0.5;
        // Use palette for ocean colors
        strip->SetPixelColor(i, applyBrightness(blendPalette(wave)));
    }
    step++;
}

void LEDEffects::effectLava() {
    for (uint16_t i = 0; i < numLeds; i++) {
        float wave = (sin((i * 0.25) + step * 0.035) + 1) / 2;
        // Use palette for lava colors
        strip->SetPixelColor(i, applyBrightness(blendPalette(wave)));
    }
    step++;
}

void LEDEffects::effectForest() {
    for (uint16_t i = 0; i < numLeds; i++) {
        float wave = (sin((i * 0.2) + step * 0.025) + 1) / 2;
        // Use palette for forest colors
        Color c = blendPalette(wave);
        if (random(1000) < 3) c = Color(255, 255, 150); // Rare firefly
        strip->SetPixelColor(i, applyBrightness(c));
    }
    step++;
}

void LEDEffects::effectStars() {
    fadeAll(4);
    if (random(100) < 12) {
        int pos = random(numLeds);
        uint8_t bright = random(180, 255);
        // Pick random color from palette
        uint8_t idx = random(paletteSize);
        strip->SetPixelColor(pos, applyBrightness(palette[idx], bright));
    }
}

// === HOLIDAY GROUP ===
void LEDEffects::effectChristmas() {
    for (uint16_t i = 0; i < numLeds; i++) {
        float pos = (i + step * 0.1);
        // Cycle through palette colors
        uint8_t idx = (int)(pos / 3) % paletteSize;
        strip->SetPixelColor(i, applyBrightness(palette[idx]));
    }
    step++;
}

void LEDEffects::effectPolice() {
    uint8_t half = numLeds / 2;
    uint8_t phase = (step / 4) % 2;
    Color c1 = palette[0];
    Color c2 = paletteSize > 1 ? palette[1] : palette[0];
    for (uint16_t i = 0; i < numLeds; i++) {
        if (i < half) {
            strip->SetPixelColor(i, phase == 0 ? applyBrightness(c1) : Color(0, 0, 0));
        } else {
            strip->SetPixelColor(i, phase == 1 ? applyBrightness(c2) : Color(0, 0, 0));
        }
    }
    step++;
}

void LEDEffects::effectRunningLights() {
    for (uint16_t i = 0; i < numLeds; i++) {
        float wave = (sin((i + step) * 0.25) + 1) / 2;
        // Blend through palette with wave modulation
        Color c = blendPalette((float)i / numLeds);
        strip->SetPixelColor(i, applyBrightness(c, wave * 255));
    }
    step++;
}
