/*
 * Buzzer.c
 *
 * Created: 25.4.2025 14.40.01
 *  Author: jesse
 */ 

#include "Buzzer.h"

void startTimer () {
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
}


//play the melody
void playMelody() {
	startTimer();
	OCR1A = 48485; //note 1
	_delay_ms(500);
	OCR1A = 30534; //note 2
	_delay_ms(500);
	OCR1A = 6944; //note 3
	_delay_ms(500);
	OCR1A = 11494; //note 4
	_delay_ms(500);
	stopTimer();
}

void stopTimer() {
	//reset all timer/counter slots
	TCCR1B = 0;
	TCCR1A = 0;
	TCNT1 = 0;
	TIMSK1 = 0;
}