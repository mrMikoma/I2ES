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
#include "channel.h"
#include "message.h"


void handle_message(uint32_t message) {
  // Control LEDs
  if (message & LED_MOVING_ON) {
    led_on(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN);
    printf("Movement LED ON\n");
  }
  if (message & LED_MOVING_OFF) {
    led_off(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN);
    printf("Movement LED OFF\n");
  }
  if (message & LED_MOVING_BLINK) {
    // Implement blinking (could use a timer interrupt)
    printf("Movement LED blinking\n");
    led_blink(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN, 3);
  }
  if (message & LED_DOOR_OPEN) {
    led_on(&DOOR_LED_PORT, DOOR_LED_PIN);
    printf("Door LED ON\n");
  }
  if (message & LED_DOOR_CLOSE) {
    led_off(&DOOR_LED_PORT, DOOR_LED_PIN);
    printf("Door LED OFF\n");
  }
  
  // Handle speaker
  if (message & SPEAKER_PLAY) {
    uint8_t sound_id = (message >> 12) & 0x0F;
    printf("Playing sound ID: ");
    USART_send_binary(sound_id);
    printf("\n");
    playMelody(sound_id);
  }
  if (message & SPEAKER_STOP) {
    //printf("Stopping sound\n");
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
    channel_slave_init(SLAVE_ADDRESS);
    
    // redirect the stdin and stdout to UART functions
    stdout = &uart_output;
    stdin = &uart_input;
    
    printf("Uno slave initialized. Listening on address: ");
    USART_send_binary(SLAVE_ADDRESS);
    printf("\n");

    volatile uint32_t received = 0x00000000; 

    while (1) 
    {      
      if (channel_available()) {

        received = channel_receive();

        printf("Received message: ");
        USART_print_binary(received, 32);
        printf("\n");

        if (is_valid_message(received)) {
          printf("Valid message detected\n");
          
          // Print control bits
          printf("Control flags: ");
          USART_print_binary((received >> 16) & 0xFFFF, 16);
          printf("\n");
          
          // Print speaker data if present
          if (received & SPEAKER_PLAY) {
            printf("Speaker data: ");
            USART_print_binary((received >> 12) & 0x0F, 4);
            printf("\n");
          }
          
          handle_message(received);
        } else {
          printf("Invalid message format\n");
        }
      }
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