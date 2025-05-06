# Introduction to Embedded Systems

Course project for BL40A1812 Introduction to Embedded Systems

## System Overview

This project is an elevator simulation system built with two Arduino boards (Uno and Mega) communicating via TWI (I2C) protocol.

### Hardware Components

- **Arduino Mega (Master)**: Handles user interface, elevator logic, LCD display (16x2), 4x4 keypad input, and emergency interrupt
- **Arduino Uno (Slave)**: Controls output devices like LEDs (movement and door indicators) and a buzzer/speaker based on commands from the Mega

### Communication Protocol

- **TWI/I2C**: The primary communication between Mega (master) and Uno (slave) at 400kHz
- **USART**: Used for debugging messages at 9600 baud with 8 data bits and 2 stop bits

### Message Protocol

- 32-bit message structure for commands between boards:
  - Bits 16-31: Control flags (16 bits)
  - Bits 12-15: Speaker data (4 bits)
  - Bits 1-11: Unused
  - Bit 0: Parity bit
- For details see: [Common/message.h](Common/message.h)

## Interrupt Usage

1. **TWI Interrupts**: Used for asynchronous message reception on the Uno (TWI_vect)
2. **Emergency Button Interrupt**: On Mega (INT3) to detect emergency button press
3. **Timer Interrupts**: Used on Uno for buzzer sound generation (TIMER2_COMPA_vect)

## Timer Usage

- **Uno Board**:
  - **Timer1 (16-bit)**: Used for tone generation in the buzzer
    - CTC mode with toggle on compare match
    - Configured with different compare values for musical notes
    - See implementation in: [Uno/Buzzer.c](Uno/Buzzer.c)
  - **Timer2 (8-bit)**: Used for note timing and duration control
    - Configured for 1ms interrupts using prescaler of 64
    - Controls how long each musical note plays
    - Handles the tempo of melodies

- **Mega Board**:
  - **No hardware timers**: The Mega implementation uses only software delays
  - **_delay_ms()**: Used for simulating elevator movement timing and door operations

- **Timer Prescaler**: Various prescalers used to achieve different frequencies for sounds on Uno

## Main Components

### Mega (Master) Implementation

- Controls the elevator state machine (IDLE, MOVING, DOOR_OPEN, EMERGENCY, FAULT)
- Reads floor selections from the 4x4 keypad
- Main implementation: [Mega/main.c](Mega/main.c)
- LCD Interface: [Mega/lcd.c](Mega/lcd.c) and [Mega/lcd.h](Mega/lcd.h)
- Keypad Interface: [Mega/keypad.c](Mega/keypad.c) and [Mega/keypad.h](Mega/keypad.h)

### Uno (Slave) Implementation

- Receives commands via TWI interrupt-driven communication
- Controls status LEDs and plays melodies through the buzzer
- Main implementation: [Uno/main.c](Uno/main.c)
- Buzzer/Speaker Control: [Uno/Buzzer.c](Uno/Buzzer.c) and [Uno/Buzzer.h](Uno/Buzzer.h)
- LED Control: [Uno/led.c](Uno/led.c) and [Uno/led.h](Uno/led.h)

### Common Code

- **TWI Library**: [Common/twi.c](Common/twi.c) and [Common/twi.h](Common/twi.h)
- **Message Protocol**: [Common/message.c](Common/message.c) and [Common/message.h](Common/message.h)
- **USART Library**: [Common/usart.c](Common/usart.c) and [Common/usart.h](Common/usart.h)

## Key Features

1. Floor selection and navigation
2. Door open/close simulation with sound effects
3. Emergency stop functionality via hardware interrupt (INT3)
4. Audio feedback for different operations (8 different melodies)
5. LED indicators for elevator status (movement and door)
6. Special easter egg melodies for certain floors (69: Rick Roll, 13: Nokia, 66: Imperial March, 93: Doom)
