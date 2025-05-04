/*
 * Buzzer.c
 *
 * Created: 25.4.2025 14.40.01
 *  Author: jesse
 */ 

#include "pins.h"
#include "Buzzer.h"
#include <stdbool.h>

// F_CPU is 16MHz, so with prescaler of 8:
// 16000000 / 8 = 2000000 timer ticks per second
// For a 1000Hz tone: 2000000 / 1000 = 2000 ticks per cycle
// Given CTC mode toggles at each compare match: 2000 / 2 = 1000 for compare register

volatile bool emergency_melody_playing = false;
volatile bool melody_playing = false;
volatile uint8_t current_note = 0;
volatile uint16_t note_counter = 0;
volatile uint16_t emergency_notes[] = {48485, 30534, 6944, 11494}; // Note values (lower = higher frequency)

void startTimer() {
	// disable interrupts
	cli();

	/* Reset variables */
	current_note = 0;
	note_counter = 0;
	
	BUZZER_DDR |= (1 << BUZZER_PIN);

	/* Set up the 16-bit timer/counter1 */
	TCNT1  = 0; //reset timer/counter register
	TCCR1B = 0; //reset timer/counter control
	TCCR1A = 0; //reset timer/counter control A
	
	TCCR1A |= (1 << 6); //set compare output mode to toggle OC1A.
	
	/* Set up waveform generation mode*/
	TCCR1A |= (1 << 0);
	TCCR1B |= (1 << 4);

	/* Enable timer/counter compare match A interrupt */
	TIMSK1 |= (1 << 1);
	
	TCCR1B |= (1 << CS10); // 0 prescaler
	
	melody_playing = true;
	
	// enable interrupts
	sei();
}

void playEmergencyMelody() {
	emergency_melody_playing = true;
	startTimer();
	OCR1A = emergency_notes[0]; // Set initial note
}

void playOpenDoorMelody() {
	emergency_melody_playing = false;
	startTimer();
	OCR1A = 48485; // Set melody tone
}

// Play the melody
void playMelody(uint8_t sound_id) {
	// we only use 4 bits from the sound_id
	sound_id = sound_id & 0x0F;

	switch (sound_id) {
		case 0:
			playEmergencyMelody();
			break;
		case 1:
			playOpenDoorMelody();
			break;
		default:
			break;
	}
}

void stopTimer() {
	// disable interrupts
	cli();

	// reset all timer/counter slots
	TCCR1B = 0;
	TCCR1A = 0;
	TCNT1 = 0;
	TIMSK1 = 0;
	
	melody_playing = false;
	emergency_melody_playing = false;

	// enable interrupts
	sei();
}

// Timer1 compare match interrupt handler
ISR(TIMER1_COMPA_vect) {
	if (!melody_playing) {
		return;
	}
	
	// Count compare match events for timing
	note_counter++;
	
	// Check if it's time to change notes (roughly 500ms)
	if (note_counter >= 1000) {
		note_counter = 0;
		
		if (emergency_melody_playing) {
			// For emergency melody, cycle through all notes
			current_note = (current_note + 1) % 4;
			OCR1A = emergency_notes[current_note];
		} else {
			// For other melodies like door open, play just one note and stop
			stopTimer();
		}
	}
}