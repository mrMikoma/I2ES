#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>
#include <stdbool.h>

/*
* Message Structure
31                              16               12              0
+--------------------------------+-------+-----------------------+
|        Control Flags           | Spkr |      Unused    |Parity |
|       (16 bits)                |(4b)  |     (11b)      | (1b)  |
+--------------------------------+-------+-----------------------+
*/

// Control bits for the message 16 bits
typedef enum {
    LED_MOVING_ON    = (1 << 15), // Bit 15: 1000 0000 0000 0000
    LED_MOVING_OFF   = (1 << 14), // Bit 14: 0100 0000 0000 0000
    LED_MOVING_BLINK = (1 << 13), // Bit 13: 0010 0000 0000 0000
    LED_DOOR_OPEN    = (1 << 12), // Bit 12: 0001 0000 0000 0000
    LED_DOOR_CLOSE   = (1 << 11), // Bit 11: 0000 1000 0000 0000
    SPEAKER_PLAY     = (1 << 10), // Bit 10: 0000 0100 0000 0000
    SPEAKER_STOP     = (1 << 9)   // Bit 09: 0000 0010 0000 0000
} MessageControlBits;

/*
* Function to check if a message is valid.
* @return 1 if valid, 0 if invalid.
*/
bool is_valid_message(uint32_t message);

/* 
 * Function to build a message with control bits and speaker data.
 * The function takes control bits and speaker data as input, 
 * constructs the message, and returns it as a 32-bit integer.
 * The parity bit is calculated and set in the least significant bit.
 * 
 * example call
 * build_message_data(LED_MOVING_ON, 0); // LED_MOVING_ON with no speaker data
 * build_message_data(LED_MOVING_OFF | SPEAKER_PLAY, 0b0001); // LED_MOVING_OFF and SPEAKER_PLAY with speaker data 0b0001
*/
uint32_t build_message_data(uint16_t control_bits, uint8_t speaker_data);
/*
 * Function to build a message with control bits only.
 * The function takes control bits as input, constructs the message,
 * and returns it as a 32-bit integer. The parity bit is calculated
 * and set in the least significant bit.
 *
 * example call
 * build_message(LED_MOVING_ON); // LED_MOVING_ON with no speaker data
*/
uint32_t build_message(uint16_t control_bits);

#endif