/*
 * led.c
 *
 * Created: 25.4.2025
 *  Author: mrMikoma
 */ 

#ifndef LED_H
#define LED_H

#define F_CPU 16000000UL

#include <avr/io.h>

void led_init(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t pin);
void led_on(volatile uint8_t *port, uint8_t pin);
void led_off(volatile uint8_t *port, uint8_t pin);
void led_toggle(volatile uint8_t *port, uint8_t pin);
void led_blink(volatile uint8_t *port, uint8_t pin, uint8_t times);

#endif
