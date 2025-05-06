#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define MOVEMENT_LED_PORT PORTB
#define MOVEMENT_LED_DDR  DDRB
#define MOVEMENT_LED_PIN  PB0

#define DOOR_LED_PORT PORTB
#define DOOR_LED_DDR  DDRB
#define DOOR_LED_PIN  PB2

#define BUZZER_DDR  DDRB
#define BUZZER_PIN  PB1

#endif // PINS_H

