#include "message.h"

#include <stdint.h>
#include <stdbool.h>

// Macro to check if two control bits are both set in the message
#define CHECK_COLLISION(msg, bit1, bit2) (((msg) & (bit1)) && ((msg) & (bit2)))

// if message has even amount of 1s, return 0 else return 1
static bool compute_parity(uint32_t msg) {
    // XOR folding for 32-bit parity calculation
    msg = msg & 0xFFFFFFFE; // Ignore parity bit
    msg ^= msg >> 16;
    msg ^= msg >> 8;
    msg ^= msg >> 4;
    msg ^= msg >> 2;
    msg ^= msg >> 1;
    return msg & 1;
}


bool is_valid_message(uint32_t message) {
    // Check mutually exclusive control bits
    if (CHECK_COLLISION(message, LED_MOVING_ON, LED_MOVING_OFF) ||
        CHECK_COLLISION(message, LED_MOVING_BLINK, LED_MOVING_ON) ||
        CHECK_COLLISION(message, LED_MOVING_BLINK, LED_MOVING_OFF) ||
        CHECK_COLLISION(message, LED_DOOR_OPEN, LED_DOOR_CLOSE)) {
        return false; // Collision detected
    }

    // Verify speaker data when PLAY is active
    uint8_t speaker_data = (message >> 12) & 0x0F;  // Bits 15-12
    if ((message & SPEAKER_PLAY) && (speaker_data == 0)) return false;

    // Get message's parity, i.e., the least significant bit (bit 0)
    bool parity_bit = message & 0x01;

    // Check if parity bit is correct
    return compute_parity(message) == parity_bit;
}


// Function to build a message with control bits and speaker data
uint32_t build_message_data(uint16_t control_bits, uint8_t speaker_data) {
    // Construct base message
    uint32_t msg = ((uint32_t)(control_bits) << 16) | ((uint32_t)(speaker_data & 0x0F) << 12);
    
    // Calculate and set parity bit (LSB)
    bool parity = compute_parity(msg);
    return msg | parity; // set final bit to parity
}

// Function to build a message with control bits only   
uint32_t build_message(uint16_t control_bits) {
    // Construct message with control bits shifted to correct position
    uint32_t msg = ((uint32_t)(control_bits) << 16);
    
    // Calculate and set parity bit (LSB)
    bool parity = compute_parity(msg);
    return msg | parity; // set final bit to parity
}