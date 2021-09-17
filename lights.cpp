#include "lights.h"
#include <Adafruit_NeoPixel.h>
#include <elapsedMillis.h>
// #include <Arduino_DebugUtils.h>

const int TIME_PER_PATTERN = 5;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

struct LightHSV {
    double hue;
    double sat;
    double val;
};

LightHSV currentLightHSV[LED_COUNT];
LightHSV transitionLightHSV[LED_COUNT];
elapsedMillis sinceUpdate;
elapsedMillis transitionTimer;
elapsedSeconds sinceChange;

LightProgram currentProgram = LightsOff;
LightProgram nextProgram;
LightPattern currentPattern;
LightPattern nextPattern;

void initializeLights() {
    strip.begin();
    for (int i = 0; i < LED_COUNT; i++) {
        currentLightHSV[i].hue = 0.0; currentLightHSV[i].sat = 0.0; currentLightHSV[i].val = 0.0;
        transitionLightHSV[i].hue = 0.0; transitionLightHSV[i].sat = 0.0; transitionLightHSV[i].val = 0.0;
    }
    strip.show();
}

void applyTransitionLights();
void applyNormalLights();
void initializeLightsOff();
void initializeRainbowChase();
void updateRainbowChase();
void initializeRainbow();
void updateRainbow();

void startLights(LightProgram program) {
    currentProgram = ProgramTransition;
    nextProgram = program;
    switch(nextProgram) {
    case LightsOff:
        initializeLightsOff();
        break;
    case WithSong:
        nextPattern = Rainbow;
        // currentPattern = Rainbow;
        initializeRainbow();
        break;
    case Standalone:
        nextPattern = RainbowChase;
        // currentPattern = RainbowChase;
        initializeRainbowChase();
        break;
    }
    // Debug.print(DBG_INFO, "curProg %u curPat %u", currentProgram, currentPattern);
    // Debug.print(DBG_INFO, "hue %u sat %u val %u", currentLightHSV[8].hue * 65535, currentLightHSV[8].sat * 255, currentLightHSV[8].val * 255);
    sinceChange = 0;
    sinceUpdate = 0;
    transitionTimer = 0;
}

void stopLights() {
    startLights(LightsOff);
}

const float updateInterval = 1000.0 / updatesPerSecond;
void updateLights() {
    if (currentProgram == LightsOff) { return; }

    if (sinceUpdate > updateInterval) {
        if (currentProgram == Standalone && sinceChange > TIME_PER_PATTERN) {
            int pattern = currentPattern + 1;
            if (pattern > Dance) {
                pattern = RainbowChase;
            }
            currentPattern = PatternTransition;
            transitionTimer = 0;
            nextPattern = static_cast<LightPattern>(pattern);
            switch(nextPattern) {
            
            case Rainbow:
                initializeRainbow(); break;
            case RainbowChase:
            default:
                initializeRainbowChase(); break;
            }
            sinceChange = 0;
        }
        if (currentPattern == PatternTransition || currentProgram == ProgramTransition) {
            applyTransitionLights();
        } else {
            applyNormalLights();
        }
        
        strip.show();
        sinceUpdate -= updateInterval;
    }
}

void applyTransitionLights() {
    // Debug.print(DBG_INFO, "transition timer %d", static_cast<long>(transitionTimer));
    if (transitionTimer > transitionTime) {
        for (int i = 0; i < LED_COUNT; i++) {
            currentLightHSV[i].hue = transitionLightHSV[i].hue;
            currentLightHSV[i].sat = transitionLightHSV[i].sat;
            currentLightHSV[i].val = transitionLightHSV[i].val;
            uint32_t rgbcolor = strip.ColorHSV(currentLightHSV[i].hue * 65535, currentLightHSV[i].sat * 255, currentLightHSV[i].val * 255);
            strip.setPixelColor(i, rgbcolor);
        }
        currentProgram = nextProgram;
        currentPattern = nextPattern;
        sinceChange = 0;
        return;
    }
    double timePercent = static_cast<double>(transitionTimer - 0) / static_cast<double>(transitionTime);
    double valuePercent = (timePercent * timePercent * (3.0 - 2.0 * timePercent));
    // Debug.print(DBG_INFO, "timePercent %d valuePercent %d", static_cast<long>(timePercent * 100), static_cast<long>(valuePercent * 100));

    for (int i = 0; i < LED_COUNT; i++) {
        double hue = currentLightHSV[i].hue + valuePercent * (transitionLightHSV[i].hue - currentLightHSV[i].hue);
        double sat = currentLightHSV[i].sat + valuePercent * (transitionLightHSV[i].sat - currentLightHSV[i].sat);
        double val = currentLightHSV[i].val + valuePercent * (transitionLightHSV[i].val - currentLightHSV[i].val);
        // if (i == 8) {
        //     Debug.print(DBG_INFO, "hue %d sat %d val %d", static_cast<long>(hue * 65535), static_cast<long>(sat * 255), static_cast<long>(val * 255));
        // }
        uint32_t rgbcolor = strip.ColorHSV(hue * 65535, sat * 255, val * 255);
        strip.setPixelColor(i, rgbcolor);
    }
}

void applyNormalLights() {
    switch(currentPattern) {
    case Rainbow:
        updateRainbow(); break;
    case RainbowChase:
    default:
        updateRainbowChase(); break;
    }

    // Debug.print(DBG_INFO, "hue %u sat %u val %u", currentLightHSV[8].hue * 65535, currentLightHSV[8].sat * 255, currentLightHSV[8].val * 255);
    
    for (int i = 0; i < LED_COUNT; i++) {
        uint32_t rgbcolor = strip.ColorHSV(currentLightHSV[i].hue * 65535, currentLightHSV[i].sat * 255, currentLightHSV[i].val * 255);
        strip.setPixelColor(i, rgbcolor);
    }
}

void initializeLightsOff() {
    for (int i = 0; i < LED_COUNT; i++) {
        // LightHSV light = transitionLightHSV[i];
        transitionLightHSV[i].hue = currentLightHSV[i].hue;
        transitionLightHSV[i].sat = 0.0;
        transitionLightHSV[i].val = 0.0;
    }
}

void initializeRainbowChase() {
    for (int i = 0; i < LED_COUNT; i++) {
        // LightHSV light = transitionLightHSV[i];
        transitionLightHSV[i].hue = (1.0 / LED_COUNT) * i;
        transitionLightHSV[i].sat = 1.0;
        transitionLightHSV[i].val = 1.0;
    }
}
const float rainbowChaseMovePerSecond = 0.2;
void updateRainbowChase() {
    double moveAmount = rainbowChaseMovePerSecond / 1000.0 * sinceUpdate;
    for (int i = 0; i < LED_COUNT; i++) {
        currentLightHSV[i].hue += moveAmount;
        if (currentLightHSV[i].hue > 1.0) {
            currentLightHSV[i].hue -= 1.0;
        }
    }
}

void initializeRainbow() {
    for (int i = 0; i < LED_COUNT; i++) {
        // LightHSV light = transitionLightHSV[i];
        transitionLightHSV[i].hue = 0.0;
        transitionLightHSV[i].sat = 1.0;
        transitionLightHSV[i].val = 1.0;
    }
}
const float rainbowMovePerSecond = 0.05;
void updateRainbow() {
    double moveAmount = rainbowMovePerSecond / 1000.0 * sinceUpdate;
    for (int i = 0; i < LED_COUNT; i++) {
        currentLightHSV[i].hue += moveAmount;
        if (currentLightHSV[i].hue > 1.0) {
            currentLightHSV[i].hue -= 1.0;
        }
    }
}

// void initializeDance() {
//     for (int i = 0; i < LED_COUNT; i++) {
//         lightHues[i] = 0;
//         lightSats[i] = 1.0;
//         lightVals[i] = 1.0;
//     }
// }
// const float rainbowMovePerSecond = 0.2;
// void updateDance() {
//     double moveAmount = rainbowMovePerSecond / 1000.0 * sinceUpdate;
//     for (int i = 0; i < LED_COUNT; i++) {
//         lightHues[i] += moveAmount;
//         if (lightHues[i] > 1.0) {
//             lightHues[i] -= 1.0;
//         }
//     }
// }
