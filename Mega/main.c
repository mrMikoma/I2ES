/*
 * Mega.c
 *
 * Created: 15/04/2025 18.43.45
 * Author : Pekka
 */ 
#define F_CPU 16000000UL
#define BAUD 9600

#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"    
#include "keypad.h"


volatile uint8_t currentFloor = 0;
volatile uint8_t selectedFloor = 0;

uint8_t requestFloorFromKeypad(uint8_t selectedFloor){
    
    uint8_t key_signal = KEYPAD_GetKey();
    
    if (key_signal != 'z' && key_signal >= '0' && key_signal <= '9') {

        selectedFloor = key_signal - '0';
        lcd_gotoxy(8,0);
        char lcd_text[4];
        itoa(selectedFloor,lcd_text,10);
        lcd_puts(lcd_text);
        //startWaitingSignal();  //odotus signaali, jos ei tule, niin jatkaa eteenpäin??? tai sitten painaa vaan jotain nappia, niin jatkuu...
        key_signal = KEYPAD_GetKey();
        if (key_signal != 'z' && key_signal >= '0' && key_signal <= '9'){
            selectedFloor = selectedFloor * 10 + key_signal - '0';
            lcd_gotoxy(8,0);

            itoa(selectedFloor,lcd_text,10);
            lcd_puts(lcd_text);
        }    
    }
    return selectedFloor;
}


void setup(){
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_puts("Starting");
	lcd_gotoxy(0,1);
	lcd_puts("Elevator!");
    KEYPAD_Init();
	_delay_ms(3000);
	lcd_clrscr();
    char lcd_text[16];
    sprintf(lcd_text,"Floor %d",currentFloor);
    itoa(selectedFloor,lcd_text,10);
    lcd_puts(lcd_text);
}

int main(void)
{
    setup();
    
    while (1) {
        selectedFloor = requestFloorFromKeypad(selectedFloor);

    }
}
