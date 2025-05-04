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

#define MELODY_EMERGENCY 0
#define MELODY_DOOR_OPEN 1
#define MELODY_DOOR_CLOSE 2
#define MELODY_HARRY_POTTER 3

#endif /* BUZZER_H_ */