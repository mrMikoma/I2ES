/*
 * Uno (Slave).c
 *
 * Created: 15/04/2025 18.44.38
 * Author : Pekka
 */ 

#define F_CPU 16000000UL
#define SLAVE_ADDRESS 0x57

// global includes
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Uno
#include "Buzzer.h"
#include "led.h"
#include "pins.h"

// Common
#include "usart.h"
#include "message.h"
#include "twi.h"

volatile bool melody_playing = false;

// Verbose debugging
void debug_twi_status(uint8_t status) {
    printf("TWI Status: 0x%02X - ", status);
    switch(status) {
        // General status codes
        case 0x00: printf("Bus error"); break;
        case 0xF8: printf("No relevant state information"); break;
        
        // Slave Receiver mode
        case 0x60: printf("Own SLA+W received, ACK sent"); break;
        case 0x68: printf("Arbitration lost, own SLA+W received, ACK sent"); break;
        case 0x70: printf("General call received, ACK sent"); break;
        case 0x78: printf("Arbitration lost, general call received, ACK sent"); break;
        case 0x80: printf("Data received, ACK sent"); break;
        case 0x88: printf("Data received, NACK sent"); break;
        case 0x90: printf("General call data received, ACK sent"); break;
        case 0x98: printf("General call data received, NACK sent"); break;
        case 0xA0: printf("STOP or REPEATED START received"); break;
        
        // Slave Transmitter mode
        case 0xA8: printf("Own SLA+R received, ACK sent"); break;
        case 0xB0: printf("Arbitration lost, own SLA+R received, ACK sent"); break;
        case 0xB8: printf("Data transmitted, ACK received"); break;
        case 0xC0: printf("Data transmitted, NACK received"); break;
        case 0xC8: printf("Last data transmitted, ACK received"); break;
        
        // Master mode status codes
        case 0x08: printf("START transmitted"); break;
        case 0x10: printf("Repeated START transmitted"); break;
        case 0x18: printf("SLA+W transmitted, ACK received"); break;
        case 0x20: printf("SLA+W transmitted, NACK received"); break;
        case 0x28: printf("Data transmitted, ACK received"); break;
        case 0x30: printf("Data transmitted, NACK received"); break;
        case 0x38: printf("Arbitration lost"); break;
        
        default: printf("Unknown status"); break;
    }
    printf("\n");
}

// This function handles incoming messages - it will be called directly from the interrupt
void handle_message(uint32_t message) {
    // Debug info
    printf("Received message: ");
    USART_print_binary(message, 32);
    printf("\n");
    
    // Check if the message is valid
    if (!is_valid_message(message)) {
        printf("Invalid message format\n");
        return;
    }
    
    printf("Valid message detected\n");
    
    // Extract control bits from the message
    uint16_t control_bits = message >> 16;
    
    // Print control bits for debugging
    printf("Control flags: ");
    USART_print_binary(control_bits, 16);
    printf("\n");
    
    // Control LEDs
    if (control_bits & LED_MOVING_ON) {
        led_on(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN);
        printf("Movement LED ON\n");
    }
    if (control_bits & LED_MOVING_OFF) {
        led_off(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN);
        printf("Movement LED OFF\n");
    }
    if (control_bits & LED_MOVING_BLINK) {
        printf("Movement LED blinking\n");
    }
    if (control_bits & LED_DOOR_OPEN) {
        led_on(&DOOR_LED_PORT, DOOR_LED_PIN);
        printf("Door LED ON\n");
    }
    if (control_bits & LED_DOOR_CLOSE) {
        led_off(&DOOR_LED_PORT, DOOR_LED_PIN);
        printf("Door LED OFF\n");
    }
    
    // Handle speaker
    if (control_bits & SPEAKER_PLAY) {
        uint8_t sound_id = (message >> 12) & 0x0F;
        printf("Playing sound ID: ");
        USART_send_binary(sound_id);
        printf("\n");
        melody_playing = true;
        playMelody(sound_id);
    }
    if (control_bits & SPEAKER_STOP) {
        melody_playing = false;
        printf("Stopping sound\n");
        stopTimer();
    }
}

// Setup the stream functions for UART, read  https://appelsiini.net/2011/simple-usart-with-avr-libc/
FILE uart_output = FDEV_SETUP_STREAM(USART_putchar, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, USART_getchar, _FDEV_SETUP_READ);

/* Main Loop */
int main(void)
{
    /* Initialize LEDs */
    led_init(&MOVEMENT_LED_DDR, &MOVEMENT_LED_PORT, MOVEMENT_LED_PIN);
    led_init(&DOOR_LED_DDR, &DOOR_LED_PORT, DOOR_LED_PIN);
    
    /* Initialize Coms */
    USART_init(9600);  // For debugging
    
    // redirect the stdin and stdout to UART functions
    stdout = &uart_output;
    stdin = &uart_input;
    
    printf("\n\n===== UNO SLAVE INITIALIZING =====\n");
    printf("Initializing slave at address: 0x%02X with interrupt support\n", SLAVE_ADDRESS);
    
    // Initialize TWI as slave device
    TWI_init_slave();
    
    // Set up the message handler callback
    TWI_set_callback(handle_message);
    
    // Enable interrupt-based message handling
    TWI_enable_interrupt(true);
    
    printf("Waiting for messages via interrupt...\n");
    printf("Current TWI status: 0x%02X\n", TWI_get_status());
    
    while (1) {

        // empty loop, we are handling messages in the interrupt
        
        // Small delay to prevent UART flooding
        _delay_ms(50);
    }
}

/* uncomment to test functionality */
// // Test all valid message combinations
// void test_valid_messages(void) {
//     // Single control flags
//     printf("Testing individual control flags:\n");
//     uint32_t msg = build_message(LED_MOVING_ON);
//     printf("LED_MOVING_ON: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     msg = build_message(LED_MOVING_OFF);
//     printf("LED_MOVING_OFF: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     msg = build_message(LED_MOVING_BLINK);
//     printf("LED_MOVING_BLINK: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     msg = build_message(LED_DOOR_OPEN);
//     printf("LED_DOOR_OPEN: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     msg = build_message(LED_DOOR_CLOSE);
//     printf("LED_DOOR_CLOSE: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     msg = build_message(SPEAKER_STOP);
//     printf("SPEAKER_STOP: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     // Test valid combinations
//     printf("\nTesting valid combinations:\n");
    
//     msg = build_message(LED_DOOR_OPEN | SPEAKER_STOP);
//     printf("LED_DOOR_OPEN | SPEAKER_STOP: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     msg = build_message_data(SPEAKER_PLAY, 1);  // Speaker play with ID 1
//     printf("SPEAKER_PLAY with ID 1: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     msg = build_message_data(LED_DOOR_CLOSE | SPEAKER_PLAY, 5);  // Multiple flags with speaker ID
//     printf("LED_DOOR_CLOSE | SPEAKER_PLAY with ID 5: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     // Test invalid combinations
//     printf("\nTesting invalid combinations (should be invalid):\n");
    
//     msg = build_message(LED_MOVING_ON | LED_MOVING_OFF);  // Mutually exclusive
//     printf("LED_MOVING_ON | LED_MOVING_OFF: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     msg = build_message(LED_DOOR_OPEN | LED_DOOR_CLOSE);  // Mutually exclusive
//     printf("LED_DOOR_OPEN | LED_DOOR_CLOSE: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     msg = build_message_data(SPEAKER_PLAY, 0);  // Invalid - speaker ID 0
//     printf("SPEAKER_PLAY with ID 0: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
    
//     // Test parity error
//     msg = build_message(LED_MOVING_ON);
//     msg ^= 0x01;  // Flip parity bit to make it invalid
//     printf("LED_MOVING_ON with incorrect parity: %s\n", is_valid_message(msg) ? "Valid" : "Invalid");
// }


// int main(void) {
//     /* Initialize LEDs */
//     led_init(&MOVEMENT_LED_DDR, &MOVEMENT_LED_PORT, MOVEMENT_LED_PIN);
//     led_init(&DOOR_LED_DDR, &DOOR_LED_PORT, DOOR_LED_PIN);
    
//     /* Initialize Coms */
//     USART_init(9600);  // For debugging
//     //TWI_init_slave_channel(SLAVE_ADDRESS);
    
//     // redirect the stdin and stdout to UART functions
//     stdout = &uart_output;
//     stdin = &uart_input;
    

//     test_valid_messages();

//     // Initialize test messages array
//     volatile uint32_t test_messages[] = {
//         build_message(LED_MOVING_ON),
//         build_message(LED_MOVING_OFF),
//         build_message(LED_MOVING_BLINK),
//         build_message(LED_DOOR_OPEN),
//         build_message(LED_DOOR_CLOSE),
//         build_message(SPEAKER_STOP),
//         build_message(LED_DOOR_OPEN | SPEAKER_STOP),
//         build_message_data(SPEAKER_PLAY, 1),  // Speaker play with ID 1
//         build_message_data(LED_DOOR_CLOSE | SPEAKER_PLAY, 5),  // Multiple flags with speaker ID
//         build_message(LED_MOVING_ON | LED_MOVING_OFF),  // Invalid: mutually exclusive
//         build_message(LED_DOOR_OPEN | LED_DOOR_CLOSE),  // Invalid: mutually exclusive
//         build_message_data(SPEAKER_PLAY, 0)  // Invalid: speaker ID 0
//     };
    
//     const uint8_t num_messages = sizeof(test_messages) / sizeof(test_messages[0]);
//     uint8_t current_message = 0;
//     volatile uint32_t received = 0;
    
//     while (1) 
//     {
//         // Get next test message
//         received = test_messages[current_message];
        
//         printf("\n\n===== Testing Message %d =====\n", current_message + 1);
//         printf("Received message: ");
//         USART_print_binary(received, 32);
//         printf("\n");

//         if (is_valid_message(received)) {
//             printf("VALID message detected\n");
            
//             // Print control bits
//             printf("Control flags: ");
//             USART_print_binary((received >> 16) & 0xFFFF, 16);
//             printf("\n");
            
//             // Print speaker data if present
//             if (received & SPEAKER_PLAY) {
//                 printf("Speaker data: ");
//                 USART_print_binary((received >> 12) & 0x0F, 4);
//                 printf("\n");
//             }
            
//             // Process the message
//             handle_message(received);
//         } else {
//             printf("INVALID message format\n");
//         }
        
//         // Move to next message
//         current_message = (current_message + 1) % num_messages;
        
//         // Delay between messages
//         _delay_ms(3000);
//     }
// }