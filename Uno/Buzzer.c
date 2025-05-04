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

// Original note values
volatile uint16_t emergency_notes[] = {48485, 30534, 6944, 11494};

// Initialize Timer1 for tone generation
void startTimer() {
	// disable interrupts
	cli();

	/* Reset variables */
	current_note = 0;
	
	/* Configure buzzer pin as output */
	BUZZER_DDR |= (1 << BUZZER_PIN);

	/* Set up the 16-bit timer/counter1 for tone generation */
	TCNT1  = 0; //reset timer/counter register
	TCCR1B = 0; //reset timer/counter control
	TCCR1A = 0; //reset timer/counter control A
	
	TCCR1A |= (1 << 6); //set compare output mode to toggle OC1A.
	
	/* Set up waveform generation mode*/
	TCCR1A |= (1 << 0);
	TCCR1B |= (1 << 4);
	
	TCCR1B |= (1 << CS10); // No prescaler
		
	startNoteTimer();
	
	// enable interrupts
	sei();
}

// Set up Timer2 for 500ms note change intervals
void startNoteTimer() {
	/* Set up Timer2 (8-bit) for note timing */
	TCNT2 = 0;    // Reset counter
	TCCR2A = 0;   // Normal mode
	TCCR2B = 0;   // Stop timer
	
	// Configure for CTC (Clear Timer on Compare) mode
	TCCR2A |= (1 << WGM21);
	
	// Set prescaler to 1024 for maximum delay
	// 16MHz / 1024 = 15.625kHz timer frequency
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
	
	// For 500ms: 15.625kHz * 0.5s = 7812.5 counts
	// 8-bit timer rolls over at 256, so we need:
	// 7812.5 / 256 = ~30.5 overflows
	// Or with OCR2A = 255: 7812.5 / 255 = ~30.6 compare matches
	// Let's use a nominal value that gives us 500ms
	OCR2A = 255;
	
	// Enable Timer2 compare interrupt
	TIMSK2 |= (1 << OCIE2A);
	
	// Global count for approximating 500ms with multiple interrupts
	current_note = 0;
}

void playEmergencyMelody() {
	OCR1A = emergency_notes[0]; // Set initial note
}

void playOpenDoorMelody() {
	OCR1A = 48485; // Set melody tone
}

void playCloseDoorMelody() {
	OCR1A = 30000; // Set melody tone
}

// Play the melody
void playMelody(uint8_t sound_id) {
	// we only use 4 bits from the sound_id
	sound_id = sound_id & 0x0F;
	melody_playing = true;
	emergency_melody_playing = false;

	switch (sound_id) {
		case 0:
			emergency_melody_playing = true;
			playEmergencyMelody();
			break;
		case 1:
			playOpenDoorMelody();
			break;
		case 2:
			playCloseDoorMelody();
		default:
			break;
	}

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
	emergency_melody_playing = false;

	// enable interrupts
	sei();
}

// Timer1 compare match interrupt handler - just for tone generation
ISR(TIMER1_COMPA_vect) {
	// This just lets the timer toggle the output pin automatically
	// No code needed here as we're using hardware PWM
}

// Timer2 compare match interrupt handler - for note timing (500ms)
ISR(TIMER2_COMPA_vect) {
	static uint8_t interrupt_count = 0;
	
	// We need multiple interrupts to reach 500ms
	// ~30 interrupts at max prescaler with OCR2A=255
	interrupt_count++;
	
	if (interrupt_count >= 30) {
		interrupt_count = 0;
		
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