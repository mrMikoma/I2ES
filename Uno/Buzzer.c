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
#include <avr/pgmspace.h> // PROGMEM support

// Melody state variables
bool melody_playing = false;
bool repeat_melody = false;
const Note* current_melody;
uint16_t melody_length;
uint16_t current_note_index = 0;
uint16_t current_duration_count = 0;
uint32_t current_note_duration_ms = 0;
uint16_t current_tempo = 120;

// Convert note frequency to timer value
uint16_t frequencyToTimerValue(uint16_t frequency) {
	if (frequency == 0) return 0; // For rests/pauses
	
	// For AVR timers in CTC mode, the formula is:
	// OCR value = (F_CPU / (2 * desired frequency)) - 1
	uint32_t calculated_ocr = (F_CPU / (2UL * frequency)) - 1;
	
	// Ensure value fits in 16-bit timer
	if (calculated_ocr > 65535) calculated_ocr = 65535;
	
	return (uint16_t)calculated_ocr;
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
const Note harry_potter_melody[] PROGMEM = {
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
const Note emergency_melody[] PROGMEM = {
	{NOTE_C5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_G5, EIGHTH},
	{NOTE_C6, QUARTER},
	{NOTE_G5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_C5, QUARTER},
	{REST, EIGHTH}
};

const Note door_open_melody[] PROGMEM = {
	{NOTE_C5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_G5, EIGHTH},
	{NOTE_C6, QUARTER}
};

const Note door_close_melody[] PROGMEM = {
	{NOTE_C6, EIGHTH},
	{NOTE_G5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_C5, QUARTER}
};

const Note nokia_melody[] PROGMEM = {
	{NOTE_E5, EIGHTH},
	{NOTE_D5, EIGHTH},
	{NOTE_FS4, QUARTER},
	{NOTE_GS4, QUARTER},
	{NOTE_CS5, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_D4, QUARTER},
	{NOTE_E4, QUARTER},
	{NOTE_B4, EIGHTH},
	{NOTE_A4, EIGHTH},
	{NOTE_CS4, QUARTER},
	{NOTE_E4, QUARTER},
	{NOTE_A4, HALF}
};

const Note never_gon_melody[] PROGMEM = {
	{NOTE_D5, DOTTED_QUARTER},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_A4, QUARTER},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_FS5, DOTTED_QUARTER},
	{NOTE_A5, SIXTEENTH},
	{NOTE_G5, SIXTEENTH},
	{NOTE_FS5, EIGHTH},
	{NOTE_D5, DOTTED_QUARTER},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_A4, HALF},
	{NOTE_A4, SIXTEENTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, EIGHTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_D5, DOTTED_QUARTER},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_A4, QUARTER},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_FS5, DOTTED_QUARTER},
	{NOTE_A5, SIXTEENTH},
	{NOTE_G5, SIXTEENTH},
	{NOTE_FS5, EIGHTH},
	{NOTE_D5, DOTTED_QUARTER},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_A4, HALF},
	{NOTE_A4, SIXTEENTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, EIGHTH},
	{NOTE_D5, SIXTEENTH},
	{REST, QUARTER},
	{NOTE_B4, EIGHTH},
	{NOTE_CS5, EIGHTH},
	{NOTE_D5, EIGHTH},
	{NOTE_D5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_CS5, DOTTED_EIGHTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_A4, HALF},
	{REST, QUARTER},
	
	{REST, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_CS5, EIGHTH},
	{NOTE_D5, EIGHTH},
	{NOTE_B4, QUARTER},
	{NOTE_A4, EIGHTH},
	{NOTE_A5, EIGHTH},
	{REST, EIGHTH},
	{NOTE_A5, EIGHTH},
	{NOTE_E5, DOTTED_QUARTER},
	{REST, QUARTER},
	{NOTE_B4, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_CS5, EIGHTH},
	{NOTE_D5, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_D5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{REST, EIGHTH},
	{REST, EIGHTH},
	{NOTE_CS5, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_A4, DOTTED_QUARTER},
	{REST, QUARTER},
	{REST, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_CS5, EIGHTH},
	{NOTE_D5, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_A4, QUARTER},
	{REST, EIGHTH},
	{NOTE_A5, EIGHTH},
	{NOTE_A5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_FS5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_D5, EIGHTH},
	
	{REST, EIGHTH},
	{NOTE_A4, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_CS5, EIGHTH},
	{NOTE_D5, EIGHTH},
	{NOTE_B4, EIGHTH},
	{REST, EIGHTH},
	{NOTE_CS5, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_A4, DOTTED_QUARTER},
	{REST, QUARTER},
	{NOTE_B4, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_CS5, EIGHTH},
	{NOTE_D5, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_A4, QUARTER},
	{REST, EIGHTH},
	{REST, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_FS5, QUARTER},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_D5, HALF},
	{NOTE_D5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_FS5, EIGHTH},
	{NOTE_E5, QUARTER},
	{NOTE_E5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_FS5, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_A4, EIGHTH},
	{NOTE_A4, QUARTER},
	
	{REST, DOTTED_QUARTER},
	{NOTE_A4, EIGHTH},
	{NOTE_B4, EIGHTH},
	{NOTE_CS5, EIGHTH},
	{NOTE_D5, EIGHTH},
	{NOTE_B4, EIGHTH},
	{REST, EIGHTH},
	{NOTE_E5, EIGHTH},
	{NOTE_FS5, EIGHTH},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_FS5, DOTTED_EIGHTH},
	{NOTE_FS5, DOTTED_EIGHTH},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_E5, DOTTED_EIGHTH},
	{NOTE_E5, DOTTED_EIGHTH},
	{NOTE_D5, DOTTED_EIGHTH},
	{NOTE_CS5, SIXTEENTH},
	{NOTE_B4, EIGHTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, QUARTER},
	{NOTE_E5, EIGHTH},
	{NOTE_CS5, DOTTED_EIGHTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_A4, QUARTER},
	{NOTE_A4, EIGHTH},
	
	{NOTE_E5, QUARTER},
	{NOTE_D5, HALF},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_FS5, DOTTED_EIGHTH},
	{NOTE_FS5, DOTTED_EIGHTH},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_A5, QUARTER},
	{NOTE_CS5, EIGHTH},
	{NOTE_D5, DOTTED_EIGHTH},
	{NOTE_CS5, SIXTEENTH},
	{NOTE_B4, EIGHTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, QUARTER},
	{NOTE_E5, EIGHTH},
	{NOTE_CS5, DOTTED_EIGHTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_A4, QUARTER},
	{NOTE_A4, EIGHTH},
	{NOTE_E5, QUARTER},
	{NOTE_D5, HALF},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	
	{NOTE_FS5, DOTTED_EIGHTH},
	{NOTE_FS5, DOTTED_EIGHTH},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_A5, QUARTER},
	{NOTE_CS5, EIGHTH},
	{NOTE_D5, DOTTED_EIGHTH},
	{NOTE_CS5, SIXTEENTH},
	{NOTE_B4, EIGHTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, QUARTER},
	{NOTE_E5, EIGHTH},
	{NOTE_CS5, DOTTED_EIGHTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_A4, QUARTER},
	{NOTE_A4, EIGHTH},
	{NOTE_E5, QUARTER},
	{NOTE_D5, HALF},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_FS5, DOTTED_EIGHTH},
	{NOTE_FS5, DOTTED_EIGHTH},
	{NOTE_E5, DOTTED_QUARTER},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	
	{NOTE_A5, QUARTER},
	{NOTE_CS5, EIGHTH},
	{NOTE_D5, DOTTED_EIGHTH},
	{NOTE_CS5, SIXTEENTH},
	{NOTE_B4, EIGHTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_D5, QUARTER},
	{NOTE_E5, EIGHTH},
	{NOTE_CS5, DOTTED_EIGHTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_A4, QUARTER},
	{NOTE_A4, EIGHTH},
	
	{NOTE_E5, QUARTER},
	{NOTE_D5, HALF},
	{REST, QUARTER}
};

const Note imperial_march_melody[] PROGMEM = {
	{NOTE_A4, DOTTED_QUARTER},
	{NOTE_A4, DOTTED_QUARTER},
	{NOTE_A4, SIXTEENTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_F4, EIGHTH},
	{REST, EIGHTH},
	{NOTE_A4, DOTTED_QUARTER},
	{NOTE_A4, DOTTED_QUARTER},
	{NOTE_A4, SIXTEENTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_F4, EIGHTH},
	{REST, EIGHTH},
	{NOTE_A4, QUARTER},
	{NOTE_A4, QUARTER},
	{NOTE_A4, QUARTER},
	{NOTE_F4, DOTTED_EIGHTH},
	{NOTE_C5, SIXTEENTH},
	
	{NOTE_A4, QUARTER},
	{NOTE_F4, DOTTED_EIGHTH},
	{NOTE_C5, SIXTEENTH},
	{NOTE_A4, HALF},
	{NOTE_E5, QUARTER},
	{NOTE_E5, QUARTER},
	{NOTE_E5, QUARTER},
	{NOTE_F5, DOTTED_EIGHTH},
	{NOTE_C5, SIXTEENTH},
	{NOTE_A4, QUARTER},
	{NOTE_F4, DOTTED_EIGHTH},
	{NOTE_C5, SIXTEENTH},
	{NOTE_A4, HALF},
	
	{NOTE_A5, QUARTER},
	{NOTE_A4, DOTTED_EIGHTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_A5, QUARTER},
	{NOTE_GS5, DOTTED_EIGHTH},
	{NOTE_G5, SIXTEENTH},
	{NOTE_DS5, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_DS5, EIGHTH},
	{REST, EIGHTH},
	{NOTE_A4, EIGHTH},
	{NOTE_DS5, QUARTER},
	{NOTE_D5, DOTTED_EIGHTH},
	{NOTE_CS5, SIXTEENTH},
	
	{NOTE_C5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_C5, SIXTEENTH},
	{REST, EIGHTH},
	{NOTE_F4, EIGHTH},
	{NOTE_GS4, QUARTER},
	{NOTE_F4, DOTTED_EIGHTH},
	{NOTE_A4, DOTTED_SIXTEENTH},
	{NOTE_C5, QUARTER},
	{NOTE_A4, DOTTED_EIGHTH},
	{NOTE_C5, SIXTEENTH},
	{NOTE_E5, HALF},
	
	{NOTE_A5, QUARTER},
	{NOTE_A4, DOTTED_EIGHTH},
	{NOTE_A4, SIXTEENTH},
	{NOTE_A5, QUARTER},
	{NOTE_GS5, DOTTED_EIGHTH},
	{NOTE_G5, SIXTEENTH},
	{NOTE_DS5, SIXTEENTH},
	{NOTE_D5, SIXTEENTH},
	{NOTE_DS5, EIGHTH},
	{REST, EIGHTH},
	{NOTE_A4, EIGHTH},
	{NOTE_DS5, QUARTER},
	{NOTE_D5, DOTTED_EIGHTH},
	{NOTE_CS5, SIXTEENTH},
	
	{NOTE_C5, SIXTEENTH},
	{NOTE_B4, SIXTEENTH},
	{NOTE_C5, SIXTEENTH},
	{REST, EIGHTH},
	{NOTE_F4, EIGHTH},
	{NOTE_GS4, QUARTER},
	{NOTE_F4, DOTTED_EIGHTH},
	{NOTE_A4, DOTTED_SIXTEENTH},
	{NOTE_A4, QUARTER},
	{NOTE_F4, DOTTED_EIGHTH},
	{NOTE_C5, SIXTEENTH},
	{NOTE_A4, HALF}
};

const Note doom_melody[] PROGMEM = {
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //1
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //5
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //9
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //13
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_FS3, DOTTED_SIXTEENTH}, {NOTE_D3, DOTTED_SIXTEENTH}, {NOTE_B2, DOTTED_SIXTEENTH}, {NOTE_A3, DOTTED_SIXTEENTH}, {NOTE_FS3, DOTTED_SIXTEENTH}, {NOTE_B2, DOTTED_SIXTEENTH}, {NOTE_D3, DOTTED_SIXTEENTH}, {NOTE_FS3, DOTTED_SIXTEENTH}, {NOTE_A3, DOTTED_SIXTEENTH}, {NOTE_FS3, DOTTED_SIXTEENTH}, {NOTE_D3, DOTTED_SIXTEENTH}, {NOTE_B2, DOTTED_SIXTEENTH},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //17
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //21
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_E3, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_E4, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_E4, DOTTED_SIXTEENTH}, {NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_G4, DOTTED_SIXTEENTH}, {NOTE_B4, DOTTED_SIXTEENTH},

	{NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_G3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, //25
	{NOTE_F3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_DS3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_F3, EIGHTH},
	{NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_G3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH},
	{NOTE_F3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_DS3, DOTTED_HALF},

	{NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_G3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, //29
	{NOTE_F3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_DS3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_F3, EIGHTH},
	{NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_G3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH},
	{NOTE_A3, DOTTED_SIXTEENTH}, {NOTE_F3, DOTTED_SIXTEENTH}, {NOTE_D3, DOTTED_SIXTEENTH}, {NOTE_A3, DOTTED_SIXTEENTH}, {NOTE_F3, DOTTED_SIXTEENTH}, {NOTE_D3, DOTTED_SIXTEENTH}, {NOTE_C4, DOTTED_SIXTEENTH}, {NOTE_A3, DOTTED_SIXTEENTH}, {NOTE_F3, DOTTED_SIXTEENTH}, {NOTE_A3, DOTTED_SIXTEENTH}, {NOTE_F3, DOTTED_SIXTEENTH}, {NOTE_D3, DOTTED_SIXTEENTH},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //33
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //37
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_CS3, EIGHTH}, {NOTE_CS3, EIGHTH}, {NOTE_CS4, EIGHTH}, {NOTE_CS3, EIGHTH}, {NOTE_CS3, EIGHTH}, {NOTE_B3, EIGHTH}, {NOTE_CS3, EIGHTH}, {NOTE_CS3, EIGHTH}, //41
	{NOTE_A3, EIGHTH}, {NOTE_CS3, EIGHTH}, {NOTE_CS3, EIGHTH}, {NOTE_G3, EIGHTH}, {NOTE_CS3, EIGHTH}, {NOTE_CS3, EIGHTH}, {NOTE_GS3, EIGHTH}, {NOTE_A3, EIGHTH},
	{NOTE_B2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_B3, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_A3, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_B2, EIGHTH},
	{NOTE_G3, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_F3, DOTTED_HALF},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //45
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_E3, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_E4, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_E4, DOTTED_SIXTEENTH}, {NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_G4, DOTTED_SIXTEENTH}, {NOTE_B4, DOTTED_SIXTEENTH},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //49
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //53
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_FS3, DOTTED_SIXTEENTH}, {NOTE_DS3, DOTTED_SIXTEENTH}, {NOTE_B2, DOTTED_SIXTEENTH}, {NOTE_FS3, DOTTED_SIXTEENTH}, {NOTE_DS3, DOTTED_SIXTEENTH}, {NOTE_B2, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_D3, DOTTED_SIXTEENTH}, {NOTE_B2, DOTTED_SIXTEENTH}, {NOTE_DS4, DOTTED_SIXTEENTH}, {NOTE_DS3, DOTTED_SIXTEENTH}, {NOTE_B2, DOTTED_SIXTEENTH},

	// -/-

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //57
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //61
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_E4, DOTTED_SIXTEENTH}, {NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_G4, DOTTED_SIXTEENTH}, {NOTE_E4, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_D4, DOTTED_SIXTEENTH}, {NOTE_E4, DOTTED_SIXTEENTH}, {NOTE_G4, DOTTED_SIXTEENTH}, {NOTE_E4, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //65
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_G3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, //69
	{NOTE_F3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_DS3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_F3, EIGHTH},
	{NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_G3, EIGHTH}, {NOTE_A2, EIGHTH}, {NOTE_A2, EIGHTH},
	{NOTE_A3, DOTTED_SIXTEENTH}, {NOTE_F3, DOTTED_SIXTEENTH}, {NOTE_D3, DOTTED_SIXTEENTH}, {NOTE_A3, DOTTED_SIXTEENTH}, {NOTE_F3, DOTTED_SIXTEENTH}, {NOTE_D3, DOTTED_SIXTEENTH}, {NOTE_C4, DOTTED_SIXTEENTH}, {NOTE_A3, DOTTED_SIXTEENTH}, {NOTE_F3, DOTTED_SIXTEENTH}, {NOTE_A3, DOTTED_SIXTEENTH}, {NOTE_F3, DOTTED_SIXTEENTH}, {NOTE_D3, DOTTED_SIXTEENTH},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //73
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //77
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //81
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, DOTTED_HALF},

	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, //73
	{NOTE_C3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_AS2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_B2, EIGHTH}, {NOTE_C3, EIGHTH},
	{NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_D3, EIGHTH}, {NOTE_E2, EIGHTH}, {NOTE_E2, EIGHTH},
	{NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_E3, DOTTED_SIXTEENTH}, {NOTE_B2, DOTTED_SIXTEENTH}, {NOTE_E3, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_C4, DOTTED_SIXTEENTH}, {NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_B3, DOTTED_SIXTEENTH}, {NOTE_G3, DOTTED_SIXTEENTH}, {NOTE_E3, DOTTED_SIXTEENTH}
};

// Initialize Timer1 for tone generation
void startTimer() {
	// disable interrupts
	cli();
	
	/* Configure buzzer pin as output */
	BUZZER_DDR |= (1 << BUZZER_PIN);
	
	/* Completely reset Timer1 */
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
	OCR1A = 0;
	
	// Get the frequency and convert it to a timer value
	Note current_note;
	// Read note from program memory
	memcpy_P(&current_note, &current_melody[current_note_index], sizeof(Note));
	uint16_t frequency = current_note.note;
	
	if (frequency != 0) {
		// Set up Timer1 in CTC mode (Clear Timer on Compare match)
		TCCR1B |= (1 << WGM12);
		
		// Configure OC1A pin to toggle on compare match
		TCCR1A |= (1 << COM1A0);
		
		// Set compare value
		OCR1A = frequencyToTimerValue(frequency);
		
		// Start timer with no prescaler for maximum frequency range
		TCCR1B |= (1 << CS10);
		
		// Make sure buzzer pin is set as output
		BUZZER_DDR |= (1 << BUZZER_PIN);
	} else {
		// This is a pause - disable the output pin
		BUZZER_DDR &= ~(1 << BUZZER_PIN);
	}
	
	// Start the timing timer separately
	startNoteTimer();
	
	// enable interrupts
	sei();
}

// Set up Timer2 for note timing - completely independent of the tone frequency
void startNoteTimer() {
	/* Set up Timer2 (8-bit) for note timing */
	TCNT2 = 0;    // Reset counter
	TCCR2A = 0;   // Normal mode
	TCCR2B = 0;   // Stop timer
	
	// Configure for CTC (Clear Timer on Compare) mode
	TCCR2A |= (1 << WGM21);
	
	// Set prescaler to 64 for 1ms precision
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

	if (melody_playing) {
		stopTimer(); // Stop any currently playing melody
	}
	
	switch (sound_id) {
		case MELODY_EMERGENCY:
			current_melody = emergency_melody;
			melody_length = sizeof(emergency_melody) / sizeof(Note);
			repeat_melody = true; // Play emergency sound forever
			current_tempo = 180; // Faster tempo for emergency
			break;
		case MELODY_DOOR_OPEN:
			current_melody = door_open_melody;
			melody_length = sizeof(door_open_melody) / sizeof(Note);
			repeat_melody = false; // Play door sound once
			current_tempo = 140; // Medium tempo for door open
			break;
		case MELODY_DOOR_CLOSE:
			current_melody = door_close_melody;
			melody_length = sizeof(door_close_melody) / sizeof(Note);
			repeat_melody = false; // Play door sound once
			current_tempo = 140; // Medium tempo for door close
			break;
		case MELODY_HARRY_POTTER:
			current_melody = harry_potter_melody;
			melody_length = sizeof(harry_potter_melody) / sizeof(Note);
			repeat_melody = false; // Play once
			current_tempo = 144;
			break;
		case MELODY_NOKIA:
			current_melody = nokia_melody;
			melody_length = sizeof(nokia_melody) / sizeof(Note);
			repeat_melody = true;
			current_tempo = 180;
			break;
		case MELODY_NEVER_GON:
			current_melody = never_gon_melody;
			melody_length = sizeof(never_gon_melody) / sizeof(Note);
			repeat_melody = true;
			current_tempo = 114;
			break;
		case MELODY_IMPERIAL_MARCH:
			current_melody = imperial_march_melody;
			melody_length = sizeof(imperial_march_melody) / sizeof(Note);
			repeat_melody = true;
			current_tempo = 120;
			break;
		case MELODY_DOOM:
			current_melody = doom_melody;
			melody_length = sizeof(doom_melody) / sizeof(Note);
			repeat_melody = true;
			current_tempo = 225;
			break;
		default:
			return; // Invalid sound ID
	}
	
	// Read the initial note from program memory
	Note current_note;
	memcpy_P(&current_note, &current_melody[current_note_index], sizeof(Note));
	
	// Calculate initial note duration
	current_note_duration_ms = calculateNoteDuration(
		current_note.duration, 
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
	// This creates the slight gap between notes for better articulation
	if (elapsed_ms >= current_note_duration_ms * 0.9) {
		// Silence the note during the remaining 10% of its duration
		// by stopping the timer, the note generator timer, remember to turn it on again when the next note starts
		TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
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
		
		// Read the new note from program memory
		Note current_note;
		memcpy_P(&current_note, &current_melody[current_note_index], sizeof(Note));
		
		// Calculate duration for the new note
		current_note_duration_ms = calculateNoteDuration(
			current_note.duration, 
			current_tempo);
		
		// Check if the next note is a pause or a normal note
		uint16_t frequency = current_note.note;
		
		// Reset Timer1 completely
		TCCR1A = 0;
		TCCR1B = 0;
		TCNT1 = 0;
		
		if (frequency != 0) {
			// Normal note - set up the timer
			
			// Configure buzzer pin as output
			BUZZER_DDR |= (1 << BUZZER_PIN);
			
			// Set up Timer1 in CTC mode
			TCCR1B |= (1 << WGM12);
			
			// Configure OC1A pin to toggle on compare match
			TCCR1A |= (1 << COM1A0);
			
			// Set compare value based on frequency
			OCR1A = frequencyToTimerValue(frequency);
			
			// Start the timer with no prescaler
			TCCR1B |= (1 << CS10);
		} else {
			// This is a pause/rest - keep the timer stopped
			// and make sure the pin is not toggling
			BUZZER_DDR &= ~(1 << BUZZER_PIN);
		}
	}
}