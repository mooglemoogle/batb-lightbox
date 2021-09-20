#include "lights.h"
#include <Adafruit_NeoPixel.h>
#include <elapsedMillis.h>
#include <math.h>
// #include <Arduino_DebugUtils.h>

const int TIME_PER_PATTERN = 60;

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
void initializeColorPulse();
void updateColorPulse();
void initializeRainbowPulse();
void updateRainbowPulse();
void initializeColorSwell();
void updateColorSwell();
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
        // nextPattern = RainbowPulse;
        // initializeRainbowPulse();
        // nextPattern = ColorSwell;
        // initializeColorSwell();
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
            if (pattern > ColorSwell) {
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
            case ColorPulse:
                initializeColorPulse(); break;
            case RainbowPulse:
                initializeRainbowPulse(); break;
            case ColorSwell:
                initializeColorSwell(); break;
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
        sinceUpdate = 0;
    }
}

double bezierBlend(double value) {
    return (value * value * (3.0 - 2.0 * value));
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
    double valuePercent = bezierBlend(timePercent);
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
    case ColorPulse:
        updateColorPulse(); break;
    case RainbowPulse:
        updateRainbowPulse(); break;
    case ColorSwell:
        updateColorSwell(); break;
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

/* Pulse Logic
Treat light strip as a circle with 16 equally spaced points at (N * TAU)/16
Each pulse has a hue/sat associated and a position around the circle.
The pulse has a width, maybe TAU/20?, we calculate each LED's distance from
the nearest pulse. If it's more than width/2 or less than -width/2, val is 0 for that pulse.
if it's within, calculate sin(dist) and use that as val with hue and sat for the pulse.

PW * PI
*/
const double RADS_PER_LED = TWO_PI/LED_COUNT;
const double LED_POS[LED_COUNT] = {
    0, 1 * RADS_PER_LED, 2 * RADS_PER_LED, 3 * RADS_PER_LED, 4 * RADS_PER_LED, 5 * RADS_PER_LED, 6 * RADS_PER_LED, 7 * RADS_PER_LED,
    8 * RADS_PER_LED, 9 * RADS_PER_LED, 10 * RADS_PER_LED, 11 * RADS_PER_LED, 12 * RADS_PER_LED, 13 * RADS_PER_LED, 14 * RADS_PER_LED, 15 * RADS_PER_LED,
};
const double pulseWidth = TWO_PI/6;
const double halfPulse = pulseWidth/2;
const double pulseRatio = 1.0 / halfPulse;
struct Pulse {
    double hue;
    double sat;
    double pos;
};
inline double PulseVal(double dist) { return bezierBlend(dist * pulseRatio); }

void updatePulsePositions(Pulse pulses[], int pulseCount, double perSecond, long millis) {
    double moveAmount = perSecond * millis / 1000.0;
    // Debug.print(DBG_INFO, "perSecond %d millis %d moveAmount %d", static_cast<uint16_t>(perSecond * 100), static_cast<uint16_t>(millis), static_cast<uint16_t>(moveAmount * 100));
    for (int i = 0; i < pulseCount; i++) {
        pulses[i].pos += moveAmount;
        if (pulses[i].pos > TWO_PI) {
            pulses[i].pos -= TWO_PI;
        } else if (pulses[i].pos < 0) {
            pulses[i].pos += TWO_PI;
        }
    }
}

void updatePulseLEDs(Pulse pulses[], int pulseCount, LightHSV lights[]) {
    for (int i = 0; i < LED_COUNT; i++) {
        const double ledPos = LED_POS[i];
        for (int j = 0; j < pulseCount; j++) {
            const double pulsePos = pulses[j].pos;
            double dist = fabs(pulsePos - ledPos);
            // if (i == 8) {
            //     Debug.print(DBG_INFO, "pulsePos %d ledPos %d", static_cast<uint16_t>(pulsePos * 1000), static_cast<uint16_t>(ledPos * 1000));
            //     Debug.print(DBG_INFO, "dist %d", static_cast<uint16_t>((pulsePos - ledPos) * 1000));
            // }
            dist = fmin(dist, fabs(dist - TWO_PI));
            // if (i == 8) {
            //     Debug.print(DBG_INFO, "dist %d", static_cast<uint16_t>(dist * 1000));
            //     Debug.print(DBG_INFO, "pulseVal %d", static_cast<uint16_t>(PulseVal(dist) * 1000));
            // }

            if (dist > halfPulse) {
                lights[i].val = 0.0;
            } else {
                lights[i].hue = pulses[j].hue;
                lights[i].sat = pulses[j].sat;
                lights[i].val = PulseVal(halfPulse - dist);
                break;
            }
        }
    }
}

inline double getRandom() {
    return static_cast <double> (rand()) / RAND_MAX;
}

const double colorPulseRadsPerSecond = PI/5;
const int colorPulseCount = 3;
const double colorPulseSpacing = TWO_PI / colorPulseCount;
double mult;
Pulse colorPulses[colorPulseCount];
void initializeColorPulse() {
    double hue = getRandom();
    double sat = getRandom() * 0.6 + 0.4;
    mult = (getRandom() > 0.5) ? 1.0 : -1.0;
    // Debug.print(DBG_INFO, "hue %d sat %d", rand1, rand2);//static_cast<long>(hue * 65535), static_cast<long>(sat * 255));
    // Debug.print(DBG_INFO, "hue %d sat %d", static_cast<uint16_t>(hue * 65535), static_cast<uint16_t>(sat * 255));
    // Debug.print(DBG_INFO, "halfPulse %d", static_cast<uint16_t>(halfPulse * 1000));

    for (int i = 0; i < colorPulseCount; i++) {
        colorPulses[i].hue = hue;
        colorPulses[i].sat = sat;
        colorPulses[i].pos = i * colorPulseSpacing;
    }

    // Debug.print(DBG_INFO, "hue %d sat %d val %d", static_cast<uint16_t>(transitionLightHSV[8].hue * 65535), static_cast<uint16_t>(transitionLightHSV[8].sat * 255), static_cast<uint16_t>(transitionLightHSV[8].val * 255));
    updatePulseLEDs(colorPulses, colorPulseCount, transitionLightHSV);
    // Debug.print(DBG_INFO, "hue %d sat %d val %d", static_cast<uint16_t>(transitionLightHSV[8].hue * 65535), static_cast<uint16_t>(transitionLightHSV[8].sat * 255), static_cast<uint16_t>(transitionLightHSV[8].val * 255));
}

void updateColorPulse() {
    updatePulsePositions(colorPulses, colorPulseCount, mult * colorPulseRadsPerSecond, sinceUpdate);
    // Debug.print(DBG_INFO, "pos %d %d %d %d", static_cast<uint16_t>(colorPulses[0].pos * 100), static_cast<uint16_t>(colorPulses[1].pos * 100), static_cast<uint16_t>(colorPulses[2].pos * 100), static_cast<uint16_t>(colorPulses[3].pos * 100));
    updatePulseLEDs(colorPulses, colorPulseCount, currentLightHSV);
    // Debug.print(DBG_INFO, "hue %d sat %d val %d", static_cast<uint16_t>(currentLightHSV[8].hue * 65535), static_cast<uint16_t>(currentLightHSV[8].sat * 255), static_cast<uint16_t>(currentLightHSV[8].val * 255));
}

const double rainbowPulseRadsPerSecond = PI/5;
const int rainbowPulseCount = 3;
const double rainbowPulseSpacing = TWO_PI / rainbowPulseCount;
const double rainbowPulseColorSpacing = (1.0 / rainbowPulseCount);
const double rainbowPulseHueMoveRatio = 0.1 / 1000.0;
Pulse rainbowPulses[rainbowPulseCount];
void initializeRainbowPulse() {
    mult = (getRandom() > 0.5) ? 1.0 : -1.0;

    for (int i = 0; i < rainbowPulseCount; i++) {
        rainbowPulses[i].hue = rainbowPulseColorSpacing * i;
        rainbowPulses[i].sat = 1.0;
        rainbowPulses[i].pos = i * rainbowPulseSpacing;
    }

    // Debug.print(DBG_INFO, "hue %d sat %d val %d", static_cast<uint16_t>(transitionLightHSV[8].hue * 65535), static_cast<uint16_t>(transitionLightHSV[8].sat * 255), static_cast<uint16_t>(transitionLightHSV[8].val * 255));
    updatePulseLEDs(rainbowPulses, rainbowPulseCount, transitionLightHSV);
    // Debug.print(DBG_INFO, "hue %d sat %d val %d", static_cast<uint16_t>(transitionLightHSV[8].hue * 65535), static_cast<uint16_t>(transitionLightHSV[8].sat * 255), static_cast<uint16_t>(transitionLightHSV[8].val * 255));
}

void updateRainbowPulse() {
    updatePulsePositions(rainbowPulses, rainbowPulseCount, mult * rainbowPulseRadsPerSecond, sinceUpdate);
    for (int i = 0; i < rainbowPulseCount; i++) {
        double hue = rainbowPulses[i].hue + (rainbowPulseHueMoveRatio * sinceUpdate);
        if (hue > 1.0) {
            hue -= 1.0;
        }
        rainbowPulses[i].hue = hue;
        // Debug.print(DBG_INFO, "hue %d pos %d", static_cast<uint16_t>(rainbowPulses[i].hue * 1000), static_cast<uint16_t>(rainbowPulses[i].pos * 1000));
    }
    updatePulseLEDs(rainbowPulses, rainbowPulseCount, currentLightHSV);
}

const double colorSwellBaseVal = 0.65;
const double colorSwellValDistance = 1.0 - colorSwellBaseVal;
const double swellsPerSecond = 0.1;
const double swellsPerSecondRatio = (swellsPerSecond * TWO_PI) / 1000.0;
void initializeColorSwell() {
    double hue = getRandom();
    double sat = getRandom() * 0.6 + 0.4;
    for (int i = 0; i < LED_COUNT; i++) {
        transitionLightHSV[i].hue = hue;
        transitionLightHSV[i].sat = sat;
        transitionLightHSV[i].val = colorSwellBaseVal;
    }
}

void updateColorSwell() {
    double currentVal = colorSwellBaseVal + (sin(millisSinceChange * swellsPerSecondRatio) * colorSwellValDistance);
    for (int i = 0; i < LED_COUNT; i++) {
        currentLightHSV[i].val = currentVal;
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
