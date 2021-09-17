#ifndef MUSIC_H_INCLUDED
#define MUSIC_H_INCLUDED

#define REST 0
#define NOTE_E4 329
#define NOTE_F4 349
#define NOTE_F4S 369
#define NOTE_G4 391
#define NOTE_A4 440
#define NOTE_B4 493
#define NOTE_C5 523
#define NOTE_C5S 554
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_F5S 739
#define NOTE_G5 783
#define NOTE_A5 880

const int tonePin = 3;
const int intervalsPerEighth = 4;

const float wholeNoteTime = 4000.0;
const float defaultBPM = 60.0;
const float bpm = 90.0;
const float bpmAdjust = defaultBPM / bpm;
const float intervalTime = (wholeNoteTime * bpmAdjust) / 8.0 / intervalsPerEighth;

bool updateSong();
void resetSong();

#endif