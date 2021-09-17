#include "music.h"
#include "Arduino.h"
#include "waveGen.h"

const int intervalsPerNote[] = {
  8 * intervalsPerEighth, // whole note
  6 * intervalsPerEighth, // extended half note
  4 * intervalsPerEighth, // half note
  3 * intervalsPerEighth, // extended quarter note
  2 * intervalsPerEighth, // quarter note
  0,
  1.5 * intervalsPerEighth, // extended eighth note
  0,
  1 * intervalsPerEighth // eighth note
};

const int numNotes = 153;

const int notes[][2] = {
  {NOTE_E4, intervalsPerNote[8]},
  {NOTE_G4, intervalsPerNote[8]},
  {NOTE_B4, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_F4, intervalsPerNote[1]},
  {REST, intervalsPerNote[6]},
  {NOTE_E4, intervalsPerNote[8]},
  {NOTE_G4, intervalsPerNote[8]},
  {NOTE_B4, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[1]},
  {REST, intervalsPerNote[6]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_F5, intervalsPerNote[8]},
  {NOTE_G5, intervalsPerNote[2]},
  {NOTE_G5, intervalsPerNote[8]},
  {NOTE_F5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[2]},
  {NOTE_F5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_A4, intervalsPerNote[8]},
  {NOTE_G4, intervalsPerNote[2]},
  {REST, intervalsPerNote[0]},
  {NOTE_E4, intervalsPerNote[8]},
  {NOTE_G4, intervalsPerNote[8]},
  {NOTE_B4, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_F4, intervalsPerNote[2]},
  {REST, intervalsPerNote[0]},
  {NOTE_E4, intervalsPerNote[8]},
  {NOTE_G4, intervalsPerNote[8]},
  {NOTE_B4, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[2]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_G5, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[2]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_B4, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_F4, intervalsPerNote[2]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_F5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[2]},
  {REST, intervalsPerNote[0]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_F5, intervalsPerNote[8]},
  {NOTE_G5, intervalsPerNote[1]},
  {REST, intervalsPerNote[6]},
  {NOTE_A4, intervalsPerNote[8]},
  {NOTE_B4, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_A5, intervalsPerNote[8]},
  {NOTE_G5, intervalsPerNote[1]},
  {REST, intervalsPerNote[6]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_F5, intervalsPerNote[8]},
  {NOTE_G5, intervalsPerNote[2]},
  {NOTE_G5, intervalsPerNote[8]},
  {NOTE_F5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[2]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_C5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[0]},
  {REST, intervalsPerNote[2]},
  {NOTE_F4S, intervalsPerNote[8]},
  {NOTE_A4, intervalsPerNote[8]},
  {NOTE_C5S, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_G4, intervalsPerNote[2]},
  {REST, intervalsPerNote[0]},
  {NOTE_F4S, intervalsPerNote[8]},
  {NOTE_A4, intervalsPerNote[8]},
  {NOTE_C5S, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[2]},
  {REST, intervalsPerNote[0]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_G5, intervalsPerNote[8]},
  {NOTE_A5, intervalsPerNote[2]},
  {NOTE_A5, intervalsPerNote[8]},
  {NOTE_G5, intervalsPerNote[8]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[2]},
  {NOTE_G5, intervalsPerNote[8]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_A4, intervalsPerNote[2]},
  {REST, intervalsPerNote[0]},
  {NOTE_F4S, intervalsPerNote[8]},
  {NOTE_A4, intervalsPerNote[8]},
  {NOTE_C5S, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_G4, intervalsPerNote[2]},
  {REST, intervalsPerNote[0]},
  {NOTE_F4S, intervalsPerNote[8]},
  {NOTE_A4, intervalsPerNote[8]},
  {NOTE_C5S, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[2]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_A5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[2]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_C5S, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_G4, intervalsPerNote[2]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_G5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[2]},
  {REST, intervalsPerNote[0]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_A5, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[2]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_C5S, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[8]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_G4, intervalsPerNote[2]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_G5, intervalsPerNote[8]},
  {NOTE_E5, intervalsPerNote[8]},
  {NOTE_F5S, intervalsPerNote[8]},
  {NOTE_D5, intervalsPerNote[1]},
};

static int currentNote = 0;
static int currentInterval = 0;

void resetSong() {
  currentInterval = 0;
  currentNote = 0;
  stopWave();
}

bool updateSong() {
  int freq = notes[currentNote][0];
  int intervals = notes[currentNote][1];

  if (currentInterval == 0) {
    if (currentNote > 0) {
      int lastFreq = notes[currentNote - 1][0];
      if (freq == lastFreq) {
        stopWave();
        delay(4);
      }
    }
    if (freq != REST) {
      startWave(freq);
    } else {
      stopWave();
    }
  }

  currentInterval += 1;
  if (currentInterval == intervals) {
    currentNote += 1;
    currentInterval = 0;

    if (currentNote == numNotes) {
      currentNote = 0;
      stopWave();
      return true;
    }
  }

  return false;
}
