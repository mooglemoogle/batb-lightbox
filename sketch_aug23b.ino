#include "music.h"
#include "lights.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <elapsedMillis.h>
#include <Arduino.h>

// #include <Arduino_DebugUtils.h>

#include "waveGen.h"

bool playingSong = false;
bool lightShow = false;

elapsedMillis sinceStart;
elapsedMillis sinceLastButton;

const int commonPin = 3;
const int lightsPin = 12;
const int soundPin = 11;
const int offPin = 10;

void checkButtons() {
  if (!digitalRead(offPin)) {
    playingSong = false;
    lightShow = false;
    resetSong();
    stopLights();
  } else if (!digitalRead(soundPin)) {
    if (!playingSong) {
      playingSong = true;
      startLights(WithSong);
      sinceStart = 0;
    }
  } else if (!digitalRead(lightsPin)) {
    if (!lightShow) {
      lightShow = true;
      if (!playingSong) {
        startLights(Standalone);
      }
    }
  }
}

unsigned long firstButton = 0;
void pressInterrupt() {
  sleep_disable();
  power_all_enable();

  detachInterrupt(digitalPinToInterrupt(commonPin));
  
  if (firstButton == 0) {
    firstButton = sinceLastButton;
  }
  else if ((sinceLastButton - firstButton) < 100) {
    return;
  }
  sinceLastButton = 0;
  firstButton = 0;

  configureDistinct();
}

void configureCommon() {
  pinMode(commonPin, INPUT_PULLUP);
  pinMode(lightsPin, OUTPUT);
  digitalWrite(lightsPin, LOW);
  pinMode(soundPin, OUTPUT);
  digitalWrite(soundPin, LOW);
  pinMode(offPin, OUTPUT);
  digitalWrite(offPin, LOW);
}

void configureDistinct() {
  pinMode(commonPin, OUTPUT);
  digitalWrite(commonPin, LOW);
  pinMode(lightsPin, INPUT_PULLUP);
  pinMode(soundPin, INPUT_PULLUP);
  pinMode(offPin, INPUT_PULLUP);
}

void setup() {
  // Serial.begin(9600);
  // Debug.timestampOn();
  initializeLights();
  configureDistinct();
  initializeWaveGen();

  lightShow = true;
  startLights(WithSong);
}

void loop() {
  updateLights();
  checkButtons();
  if (playingSong) {
    if (sinceStart >= intervalTime) {
      // Debug.print(DBG_INFO, "sinceStart %i", sinceStart);
      sinceStart -= intervalTime;
      bool done = updateSong();

      if (done) {
        playingSong = false;
        sinceLastButton = 0;
        if (lightShow) {
          startLights(Standalone);
        } else {
          stopLights();
        }
        resetSong();
      }
    }
  } else {
    if (!lightShow) {
      if (sinceLastButton > 5000) {
        configureCommon();
        attachInterrupt(digitalPinToInterrupt(commonPin), pressInterrupt, FALLING);

        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_enable();
        sleep_mode();
      }

      delay(10);
    }
  }
}
