#include "lights.h"
#include <Adafruit_NeoPixel.h>
#include <elapsedMillis.h>
#include <math.h>
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
elapsedMillis millisSinceChange;

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
void initializeDance();
void updateDance();

void startLights(LightProgram program) {
    currentProgram = ProgramTransition;
    nextProgram = program;
    switch(nextProgram) {
    case LightsOff:
        initializeLightsOff();
        break;
    case WithSong:
        nextPattern = Dance;
        initializeDance();
        break;
    case Standalone:
        nextPattern = RainbowChase;
        initializeRainbowChase();
        break;
    }
    // Debug.print(DBG_INFO, "curProg %u curPat %u", currentProgram, currentPattern);
    // Debug.print(DBG_INFO, "hue %u sat %u val %u", currentLightHSV[8].hue * 65535, currentLightHSV[8].sat * 255, currentLightHSV[8].val * 255);
    sinceChange = 0;
    millisSinceChange = 0;
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
            case Dance:
                initializeDance(); break;
            case RainbowChase:
            default:
                initializeRainbowChase(); break;
            }
            sinceChange = 0;
            millisSinceChange = 0;
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
        millisSinceChange = 0;
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
    case Dance:
        updateDance(); break;
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
        transitionLightHSV[i].hue = currentLightHSV[i].hue;
        transitionLightHSV[i].sat = 0.0;
        transitionLightHSV[i].val = 0.0;
    }
}

void initializeRainbowChase() {
    for (int i = 0; i < LED_COUNT; i++) {
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

/*
LED Locations:
0 - Left top - Cogsworth
1 - Above Ms Potts
2 - Below Ms Potts
3 - Left bottom - Rose in Jar
4 - Bottom Left - Between Jar and Beast
5 - Beast
6 - Belle
7 - Bottom Right - between Belle and book
8 - Right bottom - Book and Roses
9 - Mirror?
10 - between Mirror and Lumiere
11 - Left Top - Lumiere
12 - Top Left - Lumiere
13 - Fireworks
14 - Fireworks
15 - Top Left - Cogsworth
*/

int firework1 = 0;
int firework2 = 3;
double lastMod = 0;
void initializeDance() {
    firework1 = 0;
    firework2 = 3;
    lastMod = 0;
    transitionLightHSV[0].hue = .103;
    transitionLightHSV[0].sat = 1.0;
    transitionLightHSV[0].val = 1.0;

    transitionLightHSV[1].hue = 0.0;
    transitionLightHSV[1].sat = 0.0;
    transitionLightHSV[1].val = 0.4;

    transitionLightHSV[2].hue = 0.0;
    transitionLightHSV[2].sat = 0.0;
    transitionLightHSV[2].val = 0.4;

    transitionLightHSV[3].hue = 0.0;
    transitionLightHSV[3].sat = 1.0;
    transitionLightHSV[3].val = 0.7;

    transitionLightHSV[4].hue = .919;
    transitionLightHSV[4].sat = 1.0;
    transitionLightHSV[4].val = 0.7;

    transitionLightHSV[5].hue = 0.675;
    transitionLightHSV[5].sat = 1.0;
    transitionLightHSV[5].val = 0.7;

    transitionLightHSV[6].hue = 0.153;
    transitionLightHSV[6].sat = 1.0;
    transitionLightHSV[6].val = 0.7;

    transitionLightHSV[7].hue = 0.151;
    transitionLightHSV[7].sat = 1.0;
    transitionLightHSV[7].val = 0.7;

    transitionLightHSV[8].hue = .067;
    transitionLightHSV[8].sat = 1.0;
    transitionLightHSV[8].val = 1.0;

    transitionLightHSV[9].hue = 0.0;
    transitionLightHSV[9].sat = 0.0;
    transitionLightHSV[9].val = 0.4;

    transitionLightHSV[10].hue = 0.0;
    transitionLightHSV[10].sat = 0.0;
    transitionLightHSV[10].val = 0.4;

    transitionLightHSV[11].hue = 0.167;
    transitionLightHSV[11].sat = 1.0;
    transitionLightHSV[11].val = 1.0;

    transitionLightHSV[12].hue = .167;
    transitionLightHSV[12].sat = 1.0;
    transitionLightHSV[12].val = 1.0;

    transitionLightHSV[13].hue = 0.803;
    transitionLightHSV[13].sat = 1.0;
    transitionLightHSV[13].val = 0.5;

    transitionLightHSV[14].hue = 0.3;
    transitionLightHSV[14].sat = 1.0;
    transitionLightHSV[14].val = 0.5;

    transitionLightHSV[15].hue = 0.103;
    transitionLightHSV[15].sat = 1.0;
    transitionLightHSV[15].val = 1.0;
}

const double radsPerSecond = PI;
const double millisPerRad = 1000.0f / radsPerSecond;
const double spreadPerPulse = 0.3;
const double fireworkPulse = 0.5;
const double fireworkHues[6] = {0.803, 0.3, 0.1, 0.675, 0.78, 0.51};
void updateDance() {
    double radians = millisSinceChange / millisPerRad;
    double pulsePos = sin(radians);
    double valChange = pulsePos * spreadPerPulse;

    currentLightHSV[3].val = 0.7 + valChange; //Rose
    currentLightHSV[4].val = 0.7 - valChange; //Next to beast
    currentLightHSV[5].val = 0.7 + valChange; //Beast
    currentLightHSV[6].val = 0.7 + valChange; //Belle
    currentLightHSV[7].val = 0.7 - valChange; //Next to Belle

    double fireworks = pulsePos * fireworkPulse;
    double thisMod = fmod(radians + HALF_PI, TWO_PI);
    if (thisMod < lastMod) {
        firework1 += 1;
        if (firework1 > 5) firework1 = 0;
    } else if (thisMod >= PI && lastMod < PI) {
        firework2 += 1;
        if (firework2 > 5) firework2 = 0;
    }
    lastMod = thisMod;
    currentLightHSV[13].hue = fireworkHues[firework1];
    currentLightHSV[13].val = 0.5 + fireworks;
    currentLightHSV[14].hue = fireworkHues[firework2];
    currentLightHSV[14].val = 0.5 - fireworks;
}
