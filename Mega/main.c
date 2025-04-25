/*
 * Mega.c
 *
 * Created: 15/04/2025 18.43.45
 * Authors : Pekka, mrMikoma
 */ 

 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <util/delay.h>
 #include <stdio.h>
 #include "lcd.h"
 #include "keypad.h"
 #include "buzzer.h"
 
/* Define MEGA Output Pins */
#define EMERGENCY_INT_DDR  DDRD
#define EMERGENCY_INT_PORT PORTD 
#define EMERGENCY_INT_PIN  PD2

/* Define UNO Output Pins */
#define MOVEMENT_LED_PORT PORTB
#define MOVEMENT_LED_DDR  DDRB
#define MOVEMENT_LED_PIN  PB0

#define DOOR_LED_PORT PORTB
#define DOOR_LED_DDR  DDRB
#define DOOR_LED_PIN  PB1

/* State Management */
typedef enum {
    IDLE,
    MOVING,
    DOOR_OPEN,
    EMERGENCY,
    FAULT
} ElevatorState;

volatile ElevatorState state = IDLE;
volatile uint8_t currentFloor = 0;
volatile uint8_t selectedFloor = 0;
volatile uint8_t emergencyActivated = 0;

/* Helper Functions */
void go_to_floor(uint8_t floor) {
    // CALL UNO: led_on(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN);
    char msg[16];

    while (currentFloor != floor) {
        if (emergencyActivated) return;
        if (floor > currentFloor) {
            currentFloor++;
        } else {
            currentFloor--;
        }
        lcd_clrscr();
        sprintf(msg, "Floor: %d", currentFloor);
        lcd_write(msg);
        _delay_ms(1000);  // Simulate travel time
    }

    // CALL UNO: led_off(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN);
}

void door_sequence() {
    // CALL UNO: led_on(&DOOR_LED_PORT, DOOR_LED_PIN);
    lcd_clrscr();
    lcd_write("Door Opening...");
    _delay_ms(5000); // Simulate door open time
    lcd_clrscr();
    lcd_write("Door Closed");
    // CALL UNO: led_off(&DOOR_LED_PORT, DOOR_LED_PIN);
    _delay_ms(1000); // Simulate door closed time
}

void handle_emergency() {
    lcd_clrscr();
    lcd_write("EMERGENCY");
    // CALL UNO: blink_led(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN, 3, 400);

    while (1) {
        if (keypad_get_key()) {
            lcd_clrscr();
            lcd_write("Door Opening");
            door_sequence();
            // CALL UNO: play_emergency_melody();
            while (!keypad_get_key()); // Wait for another key to stop melody
            // CALL UNO: stop_melody();
            break;
        }
    }

    emergencyActivated = 0;
    state = IDLE;
}

// PEKALLE ?
// /* Initialize Emergency Interrupt */
// void init_emergency_interrupt() {
//     EMERGENCY_INT_DDR &= ~(1 << EMERGENCY_INT_PIN); // Input
//     EMERGENCY_INT_PORT |= (1 << EMERGENCY_INT_PIN); // Pull-up
//     EIMSK |= (1 << INT0);     // Enable INT0
//     EICRA |= (1 << ISC01);    // Trigger on falling edge
//     sei();                    // Global interrupt enable
// }
// 
// /* Interrupt Service Routine for Emergency Button */
// ISR(INT0_vect) {
//     emergencyActivated = 1;
//     state = EMERGENCY;
// }

/* Main loop */
int main(void) {
    /* Initialize LCD */
    lcd_init();
    lcd_clrscr();
    lcd_write("Choose the floor");

    /* Initialize Keypad */


    /* Initialize LEDs in UNO */
    // CALL UNO: led_init(&MOVEMENT_LED_DDR, MOVEMENT_LED_PIN);
    // CALL UNO: led_init(&DOOR_LED_DDR, DOOR_LED_PIN);

    /* Initialize Buzzer in UNO */


    /* Main Loop */
    while (1) {
        switch (state) {
            case IDLE:
                lcd_clrscr();
                lcd_write("Choose the floor");
                selectedFloor = keypad_get_key();

                if (selectedFloor == currentFloor) {
                    state = FAULT;
                } else {
                    state = MOVING;
                }
                break;

            case MOVING:
                go_to_floor(selectedFloor);
                if (!emergencyActivated) {
                    state = DOOR_OPEN;
                }
                break;

            case DOOR_OPEN:
                door_sequence();
                state = IDLE;
                break;

            case EMERGENCY:
                handle_emergency();
                break;

            case FAULT:
                lcd_clrscr();
                lcd_write("Same Floor Error");
                // CALL UNO: blink_led(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN, 3, 300);
                _delay_ms(1000); // Simulate error indication
                state = IDLE;
                break;
        }
    }
}
