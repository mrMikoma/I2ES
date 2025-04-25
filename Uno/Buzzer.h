/*
 * Buzzer.h
 *
 * Created: 25.4.2025 14.35.42
 *  Author: jesse
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void startTimer(void);

void playMelody(void);

void stopTimer(void);



#endif /* INCFILE1_H_ */