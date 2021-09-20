#ifndef LIGHTS_H_INCLUDED
#define LIGHTS_H_INCLUDED

#define LED_PIN    4
#define LED_COUNT 16

#define updatesPerSecond 60
#define transitionTime 1000

enum LightProgram { WithSong, Standalone, ProgramTransition, LightsOff };
enum LightPattern { RainbowChase, Rainbow, ColorPulse, RainbowPulse, ColorSwell, Dance, PatternTransition };

void initializeLights();
void startLights(LightProgram program);
void updateLights();
void stopLights();

#endif