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
        led_blink(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN, 3);
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
        playMelody(sound_id);
    }
    if (control_bits & SPEAKER_STOP) {
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