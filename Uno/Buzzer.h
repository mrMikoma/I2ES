/*
 * Buzzer.h
 *
 * Created: 25.4.2025 14.35.42
 *  Author: jesse
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void startTimer(void);

void playMelody(uint8_t sound_id);

void stopTimer(void);



#endif /* INCFILE1_H_ */