/*
 * Buzzer.h
 *
 * Created: 25.4.2025 14.35.42
 *  Author: jesse
 */ 


#ifndef BUZZER_H_
#define BUZZER_H_

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>

// Define a structure to hold note and duration
typedef struct {
    uint16_t note;     // Note frequency (0 = pause/silence)
    int8_t duration;   // Duration enum from NoteDuration
} Note;

// Function declarations
uint16_t frequencyToTimerValue(uint16_t frequency);
uint32_t calculateNoteDuration(int8_t duration, uint16_t tempo);

void startTimer(void);
void startNoteTimer(void);
void playMelody(uint8_t sound_id);
void stopTimer(void);

#define MELODY_EMERGENCY 0  // Emergency sound pattern
#define MELODY_DOOR_OPEN 1  // Door opening sound
#define MELODY_DOOR_CLOSE 2 // Door closing sound
#define MELODY_HARRY_POTTER 3 // Harry Potter theme
#define MELODY_NOKIA 4 // Nokia ringtone
#define MELODY_NEVER_GON 5 // Never Gonna Give You Up
#define MELODY_IMPERIAL_MARCH 6 // Imperial March
#define MELODY_DOOM 7 // Doom theme

#endif /* BUZZER_H_ */