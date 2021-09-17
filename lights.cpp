#include "lights.h"
#include <Adafruit_NeoPixel.h>
#include <elapsedMillis.h>

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
        LightHSV curLight = currentLightHSV[i];
        curLight.hue = 0; curLight.sat = 1.0; curLight.val = 1.0;
        uint32_t rgbcolor = strip.ColorHSV(curLight.hue * 65535, curLight.sat * 255, curLight.val * 255);
        strip.setPixelColor(i, rgbcolor);
        LightHSV transLight = transitionLightHSV[i];
        transLight.hue = 0; transLight.sat = 0; transLight.val = 0;
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
    // currentProgram = ProgramTransition;
    // nextProgram = program;
    currentProgram = program;
    switch(currentProgram) {
    case LightsOff:
        initializeLightsOff();
        break;
    case WithSong:
        // nextPattern = Rainbow;
        currentPattern = Rainbow;
        initializeRainbow();
        break;
    case Standalone:
        // nextPattern = RainbowChase;
        currentPattern = RainbowChase;
        initializeRainbowChase();
        break;
    }
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
    if (transitionTimer > transitionTime) {
        for (int i = 0; i < LED_COUNT; i++) {
            LightHSV curLight = currentLightHSV[i];
            LightHSV transLight = transitionLightHSV[i];
            curLight.hue = transLight.hue;
            curLight.sat = transLight.sat;
            curLight.val = transLight.val;
        }
        currentProgram = nextProgram;
        currentPattern = nextPattern;
        sinceChange = 0;
        return;
    }
    double timePercent = transitionTimer / transitionTime;
    double valuePercent = (timePercent * timePercent * (3.0 - 2.0 * timePercent));

    for (int i = 0; i < LED_COUNT; i++) {
        LightHSV lastLight = currentLightHSV[i];
        LightHSV nextLight = transitionLightHSV[i];
        double hue = lastLight.hue + valuePercent * (nextLight.hue - lastLight.hue);
        double sat = lastLight.sat + valuePercent * (nextLight.sat - lastLight.sat);
        double val = lastLight.val + valuePercent * (nextLight.val - lastLight.val);
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
    for (int i = 0; i < LED_COUNT; i++) {
        LightHSV light = currentLightHSV[i];
        uint32_t rgbcolor = strip.ColorHSV(light.hue * 65535, light.sat * 255, light.val * 255);
        strip.setPixelColor(i, rgbcolor);
    }
}

void initializeLightsOff() {
    for (int i = 0; i < LED_COUNT; i++) {
        // LightHSV light = transitionLightHSV[i];
        LightHSV light = currentLightHSV[i];
        light.sat = 0.0;
        light.val = 0.0;
    }
}

void initializeRainbowChase() {
    for (int i = 0; i < LED_COUNT; i++) {
        // LightHSV light = transitionLightHSV[i];
        LightHSV light = currentLightHSV[i];
        light.hue = (1.0 / LED_COUNT) * i;
        light.sat = 1.0;
        light.val = 1.0;
    }
}
const float rainbowChaseMovePerSecond = 0.2;
void updateRainbowChase() {
    double moveAmount = rainbowChaseMovePerSecond / 1000.0 * sinceUpdate;
    for (int i = 0; i < LED_COUNT; i++) {
        LightHSV light = currentLightHSV[i];
        light.hue += moveAmount;
        if (light.hue > 1.0) {
            light.hue -= 1.0;
        }
    }
}

void initializeRainbow() {
    for (int i = 0; i < LED_COUNT; i++) {
        // LightHSV light = transitionLightHSV[i];
        LightHSV light = currentLightHSV[i];
        light.hue = 0.0;
        light.sat = 1.0;
        light.val = 1.0;
    }
}
const float rainbowMovePerSecond = 0.05;
void updateRainbow() {
    double moveAmount = rainbowMovePerSecond / 1000.0 * sinceUpdate;
    for (int i = 0; i < LED_COUNT; i++) {
        LightHSV light = currentLightHSV[i];
        light.hue += moveAmount;
        if (light.hue > 1.0) {
            light.hue -= 1.0;
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
