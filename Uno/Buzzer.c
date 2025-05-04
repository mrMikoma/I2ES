/*
 * Buzzer.c
 *
 * Created: 25.4.2025 14.40.01
 *  Author: jesse
 */ 

#include "Buzzer.h"
#include <stdbool.h>

volatile bool emergency_melody_playing = false;
volatile bool melody_playing = false;

void startTimer () {

	// disable interrupts
	cli();

	/* Set up the 16-bit timer/counter1 */
	TCNT1  = 0; //reset timer/counter register
	TCCR1B = 0; //reset timer/counter control
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
	while (emergency_melody_playing) {
		OCR1A = 48485; //note 1
		_delay_ms(500);
		OCR1A = 30534; //note 2
		_delay_ms(500);
		OCR1A = 6944; //note 3
		_delay_ms(500);
		OCR1A = 11494; //note 4
		_delay_ms(500);
	}
}

void playOpenDoorMelody() {
	OCR1A = 48485; //note 1
	_delay_ms(500);
}

//play the melody
void playMelody(uint8_t sound_id) {

	// we only use 4 bits from the sound_id
	sound_id = sound_id & 0x0F;

	startTimer();

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
	stopTimer();
}

void stopTimer() {

	// disable interrupts
	cli();

	//reset all timer/counter slots
	TCCR1B = 0;
	TCCR1A = 0;
	TCNT1 = 0;
	TIMSK1 = 0;
	melody_playing = false;
	emergency_melody_playing = false;	

	// enable interrupts
	sei();
}