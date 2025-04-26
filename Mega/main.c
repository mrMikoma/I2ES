/*
 * Mega.c
 *
 * Created: 15/04/2025 18.43.45
 * Authors : Pekka, mrMikoma
 */ 
#define F_CPU 16000000UL
#define BAUD 9600

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"    
#include "keypad.h"
 
/* Define MEGA Output Pins */
#define EMERGENCY_INT_DDR  DDRD
#define EMERGENCY_INT_PORT PORTD 
#define EMERGENCY_INT_PIN  PD3

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


uint8_t requestFloorFromKeypad(uint8_t selectedFloor){
    
    uint8_t key_signal = KEYPAD_GetKey();
    
    if (key_signal != 'z' && key_signal >= '0' && key_signal <= '9') {

        selectedFloor = key_signal - '0';
        lcd_gotoxy(0,0);
	    char lcd_text[16];
		sprintf(lcd_text,"Floor:%02d Sel:%02d",currentFloor,selectedFloor);
		lcd_puts(lcd_text);
        //startWaitingSignal();  //odotus signaali, jos ei tule, niin jatkaa eteenp�in??? tai sitten painaa vaan jotain nappia, niin jatkuu...
        key_signal = KEYPAD_GetKey();
        if (key_signal != 'z' && key_signal >= '0' && key_signal <= '9'){
            selectedFloor = selectedFloor * 10 + key_signal - '0';
            char lcd_text[16];
			sprintf(lcd_text,"Floor:%02d Sel:%02d",currentFloor,selectedFloor);
			lcd_gotoxy(0,0);
			lcd_puts(lcd_text);
        }    
    }
    return selectedFloor;
}

/* Helper Functions */
void go_to_floor(uint8_t floor) {
    // CALL UNO: led_on(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN);
    char msg[16];

    while (currentFloor != floor) {
        if (emergencyActivated) return;
        if (floor > currentFloor) {
			lcd_gotoxy(0,1);
			lcd_puts("Moving up       ");
            currentFloor++;
        } else {
			lcd_gotoxy(0,1);
            lcd_puts("Moving down     ");
			currentFloor--;
        }
        lcd_gotoxy(0,0);
        sprintf(msg, "Floor:%02d", currentFloor);
        lcd_puts(msg);
        _delay_ms(1000);  // Simulate travel time
	}
	
}
void setup(){
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_puts("Starting");
	lcd_gotoxy(0,1);
	lcd_puts("Elevator!");
    KEYPAD_Init();
	_delay_ms(1000);
	lcd_clrscr();
    char lcd_text[16];
    sprintf(lcd_text,"Floor %02d",currentFloor);
    itoa(selectedFloor,lcd_text,10);
    lcd_puts(lcd_text);
}

    // CALL UNO: led_off(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN);


void door_sequence() {
    // CALL UNO: led_on(&DOOR_LED_PORT, DOOR_LED_PIN);
    lcd_gotoxy(0,1);
	
    lcd_puts("Door Opening... ");
    _delay_ms(5000); // Simulate door open time
    lcd_gotoxy(0,1);
    lcd_puts("Door Closed     ");
    // CALL UNO: led_off(&DOOR_LED_PORT, DOOR_LED_PIN);
    _delay_ms(1000); // Simulate door closed time
}

void handle_emergency() {
    lcd_gotoxy(0,0);
    lcd_puts("   EMERGENCY!   ");
    lcd_gotoxy(0,1);
    lcd_puts("Press any Button");
    // CALL UNO: blink_led(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN, 3, 400);

    while (1) {
        if (KEYPAD_GetKey()) {
            door_sequence();
            lcd_gotoxy(0,1);
            lcd_puts("Press any Button");
            // CALL UNO: play_emergency_melody();
            while (!KEYPAD_GetKey()); // Wait for another key to stop melody
            // CALL UNO: stop_melody();
            break;
        }
    }

    emergencyActivated = 0;
    state = IDLE;
    selectedFloor = currentFloor;
}

// PEKALLE ?
 /* Initialize Emergency Interrupt */
void init_emergency_interrupt() {
    EMERGENCY_INT_DDR &= ~(1 << PD3); // clears the bit, setting the pin as an input.
    EMERGENCY_INT_PORT |= (1 << PD3); // enables the internal pull-up resistor, keeping the pin HIGH when idle.
    EICRA |= (1 << ISC31);    // Trigger on FALLING edge (HIGH --> LOW)
    EICRA |= (1 << ISC30);    // combination 1,1 sets button to trigger when LOW --> HIGH
  
	EIMSK |= (1 << INT3);     // Enable INT3 interrupt

	sei();                    // Enable global interrupts
}
 
 /* Interrupt Service Routine for Emergency Button */
ISR(INT3_vect) {
    emergencyActivated = 1;
    state = EMERGENCY;
}

/* Main loop */
int main(void) {
	setup();
	init_emergency_interrupt();

    /* Initialize LEDs in UNO */
    // CALL UNO: led_init(&MOVEMENT_LED_DDR, MOVEMENT_LED_PIN);
    // CALL UNO: led_init(&DOOR_LED_DDR, DOOR_LED_PIN);

    /* Initialize Buzzer in UNO */


    /* Main Loop */
    while (1) {
		lcd_clrscr();

	    char lcd_text[16];
		sprintf(lcd_text,"Floor:%02d Sel:%02d",currentFloor,selectedFloor);
		lcd_puts(lcd_text);
		
        switch (state) {
            case IDLE:
                //lcd_gotoxy(11,0);
                
                selectedFloor = requestFloorFromKeypad(selectedFloor);
                if (emergencyActivated)break;
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
                selectedFloor = currentFloor;
                break;

            case EMERGENCY:
                handle_emergency();
                break;

            case FAULT:
                lcd_clrscr();
                lcd_puts("Same Floor Error");
                // CALL UNO: blink_led(&MOVEMENT_LED_PORT, MOVEMENT_LED_PIN, 3, 300);
                _delay_ms(1000); // Simulate error indication
                state = IDLE;
                break;
        }
    }
}
