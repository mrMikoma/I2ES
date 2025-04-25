/*
 * led.c
 *
 * Created: 25.4.2025
 *  Author: mrMikoma
 */ 

 #include "led.h"
 #include <util/delay.h>
 
 void led_init(volatile uint8_t *ddr, volatile uint8_t *port, uint8_t pin) {
    *ddr |= (1 << pin);
    *port &= ~(1 << pin);
}

void led_on(volatile uint8_t *port, uint8_t pin) {
    *port |= (1 << pin);
}

void led_off(volatile uint8_t *port, uint8_t pin) {
    *port &= ~(1 << pin);
}

void led_toggle(volatile uint8_t *port, uint8_t pin) {
    *port ^= (1 << pin);
}

void led_blink(volatile uint8_t *port, uint8_t pin, uint8_t times, uint16_t delay_ms) {
    for (uint8_t i = 0; i < times; i++) {
        led_on(port, pin);
        _delay_ms(delay_ms);
        led_off(port, pin);
        _delay_ms(delay_ms);
    }
}
