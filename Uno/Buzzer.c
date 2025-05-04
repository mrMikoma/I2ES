/*
 * Buzzer.c
 *
 * Created: 25.4.2025 14.40.01
 *  Author: jesse
 */ 

/* 
  Code modified from:
  Hedwig's theme - Harry Potter 
  Connect a piezo buzzer or speaker to pin 11 or select a new pin.
  More songs available at https://github.com/robsoncouto/arduino-songs                                            
                                              
                                              Robson Couto, 2019
*/

#include "pins.h"
#include "Buzzer.h"
#include "notes.h"
#include <stdbool.h>
#include <stdlib.h> // For abs()

// Define a structure to hold note and duration
typedef struct {
	uint16_t note;     // Note frequency (0 = pause/silence)
	int8_t duration;   // Duration enum from NoteDuration
} Note;

// Convert note frequency to timer value
uint16_t frequencyToTimerValue(uint16_t frequency) {
	if (frequency == 0) return 0; // For rests/pauses
	return F_CPU / (2UL * frequency);
}

// Calculate note duration in milliseconds based on tempo
uint32_t calculateNoteDuration(int8_t duration, uint16_t tempo) {
	uint32_t wholenote = (60000UL * 4) / tempo;
	uint32_t ms;
	
	if (duration > 0) {
		// Regular note duration
		ms = wholenote / duration;
	} else if (duration < 0) {
		// Dotted note (1.5x duration)
		ms = wholenote / abs(duration);
		ms *= 1.5;
	} else {
		// Should never happen, but default to quarter note
		ms = wholenote / 4;
	}
	
	return ms;
}

// Harry Potter Theme (Hedwig's Theme) melody
const Note harry_potter_melody[] = {
	{REST, HALF},
	{NOTE_D4, QUARTER},
	{NOTE_G4, DOTTED_QUARTER},
	{NOTE_AS4, EIGHTH},
	{NOTE_A4, QUARTER},
	{NOTE_G4, HALF},
	{NOTE_D5, QUARTER},
	{NOTE_C5, DOTTED_HALF},
	{NOTE_A4, DOTTED_HALF},
	{NOTE_G4, DOTTED_QUARTER},
	{NOTE_AS4, EIGHTH},
	{NOTE_A4, QUARTER},
	{NOTE_F4, HALF},
	{NOTE_GS4, QUARTER},
	{NOTE_D4, DOTTED_WHOLE},
	{NOTE_D4, QUARTER},

	// Measure 10
	{NOTE_G4, DOTTED_QUARTER},
	{NOTE_AS4, EIGHTH},
	{NOTE_A4, QUARTER},
	{NOTE_G4, HALF},
	{NOTE_D5, QUARTER},
	{NOTE_F5, HALF},
	{NOTE_E5, QUARTER},
	{NOTE_DS5, HALF},
	{NOTE_B4, QUARTER},
	{NOTE_DS5, DOTTED_QUARTER},
	{NOTE_D5, EIGHTH},
	{NOTE_CS5, QUARTER},
	{NOTE_CS4, HALF},
	{NOTE_B4, QUARTER},
	{NOTE_G4, DOTTED_WHOLE},
	{NOTE_AS4, QUARTER},

	// Measure 18
	{NOTE_D5, HALF},
	{NOTE_AS4, QUARTER},
	{NOTE_D5, HALF},
	{NOTE_AS4, QUARTER},
	{NOTE_DS5, HALF},
	{NOTE_D5, QUARTER},
	{NOTE_CS5, HALF},
	{NOTE_A4, QUARTER},
	{NOTE_AS4, DOTTED_QUARTER},
	{NOTE_D5, EIGHTH},
	{NOTE_CS5, QUARTER},
	{NOTE_CS4, HALF},
	{NOTE_D4, QUARTER},
	{NOTE_D5, DOTTED_WHOLE},
	{REST, QUARTER},
	{NOTE_AS4, QUARTER},
};

// Original basic melodies
const Note emergency_melody[] = {
	{NOTE_G5, QUARTER},
	{NOTE_C5, QUARTER},
	{NOTE_G4, QUARTER},
	{NOTE_C4, QUARTER}
};

const Note door_open_melody[] = {
	{NOTE_C5, QUARTER}
};

const Note door_close_melody[] = {
	{NOTE_G4, QUARTER}
};

// Melody state variables
volatile bool melody_playing = false;
volatile bool repeat_melody = false;
volatile const Note* current_melody;
volatile uint8_t melody_length;
volatile uint8_t current_note_index = 0;
volatile uint8_t current_duration_count = 0;
volatile uint32_t current_note_duration_ms = 0;
volatile uint16_t current_tempo = 120;

// Initialize Timer1 for tone generation
void startTimer() {
	// disable interrupts
	cli();
	
	/* Configure buzzer pin as output */
	BUZZER_DDR |= (1 << BUZZER_PIN);

	/* Set up the 16-bit timer/counter1 for tone generation */
	TCNT1  = 0; // Reset timer/counter register
	TCCR1B = 0; // Reset timer/counter control
	TCCR1A = 0; // Reset timer/counter control A
	
	// Get the frequency and convert it to a timer value
	uint16_t frequency = current_melody[current_note_index].note;
	uint16_t timer_value = frequencyToTimerValue(frequency);
	
	if (frequency != 0) {
		// Normal note - set up to toggle pin
		TCCR1A |= (1 << 6); // Set compare output mode to toggle OC1A.
		
		/* Set up waveform generation mode */
		TCCR1A |= (1 << 0);
		TCCR1B |= (1 << 4);
		
		// Set the timer value
		OCR1A = timer_value;
		
		TCCR1B |= (1 << CS10); // No prescaler
	} else {
		// This is a pause - disable timer output
		BUZZER_DDR &= ~(1 << BUZZER_PIN); // Set pin as input to disable output
	}
	
	startNoteTimer();
	
	// enable interrupts
	sei();
}

// Set up Timer2 for note timing
void startNoteTimer() {
	/* Set up Timer2 (8-bit) for note timing */
	TCNT2 = 0;    // Reset counter
	TCCR2A = 0;   // Normal mode
	TCCR2B = 0;   // Stop timer
	
	// Configure for CTC (Clear Timer on Compare) mode
	TCCR2A |= (1 << WGM21);
	
	// Set prescaler to 64 instead of 1024 for better timing precision
	// 16MHz / 64 = 250kHz timer frequency
	TCCR2B |= (1 << CS22);  // prescaler = 64
	
	// With 250kHz timer and OCR2A = 249:
	// 250kHz / 250 = 1000Hz = 1ms interrupt frequency
	OCR2A = 249;  // 1ms precision (250 timer ticks)
	
	// Enable Timer2 compare interrupt
	TIMSK2 |= (1 << OCIE2A);
}

// Play a specified melody
void playMelody(uint8_t sound_id) {
	// we only use 4 bits from the sound_id
	sound_id = sound_id & 0x0F;
	
	// Reset melody state
	current_note_index = 0;
	current_duration_count = 0;
	
	switch (sound_id) {
		case 0: // Emergency sound
			current_melody = emergency_melody;
			melody_length = sizeof(emergency_melody) / sizeof(Note);
			repeat_melody = true; // Play emergency sound forever
			current_tempo = 120; // Default tempo
			break;
		case 1: // Door open sound
			current_melody = door_open_melody;
			melody_length = sizeof(door_open_melody) / sizeof(Note);
			repeat_melody = false; // Play door sound once
			current_tempo = 120; // Default tempo
			break;
		case 2: // Door close sound
			current_melody = door_close_melody;
			melody_length = sizeof(door_close_melody) / sizeof(Note);
			repeat_melody = false; // Play door sound once
			current_tempo = 120; // Default tempo
			break;
		case 3: // Harry Potter Theme
			current_melody = harry_potter_melody;
			melody_length = sizeof(harry_potter_melody) / sizeof(Note);
			repeat_melody = false; // Play once
			current_tempo = 144;
			break;
		default:
			return; // Invalid sound ID
	}
	
	// Calculate initial note duration
	current_note_duration_ms = calculateNoteDuration(
		current_melody[current_note_index].duration, 
		current_tempo);
	
	melody_playing = true;
	startTimer();
}

void stopTimer() {
	// disable interrupts
	cli();

	// Stop both timers
	TCCR1B = 0;
	TCCR1A = 0;
	TCNT1 = 0;
	TIMSK1 = 0;
	
	TCCR2B = 0;
	TCCR2A = 0;
	TCNT2 = 0;
	TIMSK2 = 0;
	
	// Reset buzzer pin to ensure no sound
	BUZZER_DDR &= ~(1 << BUZZER_PIN);
	
	melody_playing = false;
	repeat_melody = false;

	// enable interrupts
	sei();
}

// Timer2 compare match interrupt handler - for note timing
ISR(TIMER2_COMPA_vect) {
	static uint32_t elapsed_ms = 0;
	
	if (!melody_playing) {
		return;
	}
	
	// Each interrupt is now exactly 1ms
	elapsed_ms += 1;
	
	// Check if we've played the current note for 90% of its duration
	if (elapsed_ms >= current_note_duration_ms * 0.9) {
		// Silence the note during the remaining 10% of its duration
		BUZZER_DDR &= ~(1 << BUZZER_PIN);
	}
	
	// Move to the next note after the full duration
	if (elapsed_ms >= current_note_duration_ms) {
		elapsed_ms = 0;
		current_note_index++;
		
		// Check if we've reached the end of the melody
		if (current_note_index >= melody_length) {
			if (repeat_melody) {
				// Loop back to the beginning of the melody
				current_note_index = 0;
			} else {
				// Stop if we don't need to repeat
				stopTimer();
				return;
			}
		}
		
		// Calculate duration for the new note
		current_note_duration_ms = calculateNoteDuration(
			current_melody[current_note_index].duration, 
			current_tempo);
		
		// Check if the next note is a pause or a normal note
		uint16_t frequency = current_melody[current_note_index].note;
		if (frequency != 0) {
			// Normal note - enable timer output
			BUZZER_DDR |= (1 << BUZZER_PIN);
			TCCR1A |= (1 << 6); // Set compare output mode to toggle OC1A
			
			// Convert frequency to timer value and set it
			OCR1A = frequencyToTimerValue(frequency);
		} else {
			// Pause - disable timer output
			BUZZER_DDR &= ~(1 << BUZZER_PIN);
		}
	}
}