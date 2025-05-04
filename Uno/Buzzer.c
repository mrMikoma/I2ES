/*
 * Buzzer.c
 *
 * Created: 25.4.2025 14.40.01
 *  Author: jesse
 */ 

#include "pins.h"
#include "Buzzer.h"
#include "notes.h"
#include <stdbool.h>

// Define a structure to hold note and duration
typedef struct {
	uint16_t note;     // Timer value for the note frequency (0 = pause/silence)
	uint32_t duration; // Duration in milliseconds
} Note;

// Melody definitions
const Note emergency_melody[] = {
	{48485, 500}, // Note 1 - 500ms
	{30534, 500}, // Note 2 - 500ms
	{6944, 500},  // Note 3 - 500ms
	{11494, 500}  // Note 4 - 500ms
};

const Note door_open_melody[] = {
	{48485, 500}  // Single note - 500ms
};

const Note door_close_melody[] = {
	{30000, 500}  // Single note - 500ms
};

// Example melody with pauses
const Note test_melody[] = {
	{48485, 300},  // Note - 300ms
	{0, 200},      // Pause - 200ms
	{30534, 300},  // Note - 300ms
	{0, 200},      // Pause - 200ms
	{6944, 500}    // Note - 500ms
};

// Melody state variables
volatile bool melody_playing = false;
volatile bool repeat_melody = false;
volatile const Note* current_melody;
volatile uint8_t melody_length;
volatile uint8_t current_note_index = 0;
volatile uint8_t current_duration_count = 0;

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
	
	if (current_melody[current_note_index].note != 0) {
		// Normal note - set up to toggle pin
		TCCR1A |= (1 << 6); // Set compare output mode to toggle OC1A.
		
		/* Set up waveform generation mode */
		TCCR1A |= (1 << 0);
		TCCR1B |= (1 << 4);
		
		// Set the initial note
		OCR1A = current_melody[current_note_index].note;
		
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
			break;
		case 1: // Door open sound
			current_melody = door_open_melody;
			melody_length = sizeof(door_open_melody) / sizeof(Note);
			repeat_melody = false; // Play door sound once
			break;
		case 2: // Door close sound
			current_melody = door_close_melody;
			melody_length = sizeof(door_close_melody) / sizeof(Note);
			repeat_melody = false; // Play door sound once
			break;
		case 3: // Test melody with pauses
			current_melody = test_melody;
			melody_length = sizeof(test_melody) / sizeof(Note);
			repeat_melody = false; // Play once
			break;
		default:
			return; // Invalid sound ID
	}
	
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

// Timer1 compare match interrupt handler - just for tone generation
//ISR(TIMER1_COMPA_vect) {
//	// This just lets the timer toggle the output pin automatically
//	// No code needed here as we're using hardware PWM
//}

// Timer2 compare match interrupt handler - for note timing
ISR(TIMER2_COMPA_vect) {
	static uint32_t elapsed_ms = 0;
	
	if (!melody_playing) {
		return;
	}
	
	// Each interrupt is now exactly 1ms
	elapsed_ms += 1;
	
	// Check if we've played the current note for its full duration
	if (elapsed_ms >= current_melody[current_note_index].duration) {
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
		
		// Check if the next note is a pause or a normal note
		if (current_melody[current_note_index].note != 0) {
			// Normal note - enable timer output
			BUZZER_DDR |= (1 << BUZZER_PIN);
			TCCR1A |= (1 << 6); // Set compare output mode to toggle OC1A
			// Set the next note
			OCR1A = current_melody[current_note_index].note;
		} else {
			// Pause - disable timer output
			BUZZER_DDR &= ~(1 << BUZZER_PIN);
		}
	}
}