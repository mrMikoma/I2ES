#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>

#include "usart.h"

#define F_CPU 16000000UL

// Debug prefix state
static bool new_line = true;

void USART_init(uint32_t baudrate) {
    // Calculate UBRR value (see datasheet section 20.11)
    uint16_t ubrr = (F_CPU / 16 / baudrate) - 1;
    
    /* Set baud rate in the USART Baud Rate Registers (UBRR) *///datasheet p.222
    UBRR0H = (uint8_t)(ubrr >> 8); //datasheet p.206, ubbr value is 103
    UBRR0L = (uint8_t)ubrr; //datasheet p.206, , ubbr value is 103
    
    /* Enable receiver and transmitter on RX0 and TX0 */
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0); //RX complete interrupt enable//Transmitter enable // datasheet p.206, p.220
    
    /* Set frame format: 8 bit data, 2 stop bit */
    UCSR0C |= (1 << USBS0) | (3 << UCSZ00);//Stop bit selection at 2-bit// UCSZ bit setting at 8 bit//datasheet p.221 and p.222

}

int USART_putchar(char c, FILE *stream) {
    // Add DEBUG: prefix at start of each line
    if (new_line) {
        USART_print_string("DEBUG: ");
        new_line = false;
    }
    
    if (c == '\n' || c == '\r') {
        // Send both CR and LF for proper line termination
        USART_transmit('\r');
        USART_transmit('\n');
        new_line = true;
    } else {
        USART_transmit(c);
    }
    
    return 0;
}

int USART_getchar(FILE *stream) {
    return USART_receive();
}

void USART_transmit(uint8_t data) {
    /* Wait until the transmit buffer is empty*///datasheet p.207, p. 219
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

void USART_print_string(const char* str) {
    while(*str) {
        USART_transmit(*str++);
    }
}

void USART_send_binary(uint8_t data) {
    // Output binary representation of byte (MSB first)
    for (int8_t i = 7; i >= 0; i--) {
        if (data & (1 << i)) {
            USART_transmit('1');
        } else {
            USART_transmit('0');
        }
    }
}

void USART_print_binary(uint32_t value, uint8_t bits) {
    // Print 'bits' number of bits from value (MSB first)
    if (new_line) {
        USART_print_string("DEBUG: ");
        new_line = false;
    }
    
    // Calculate starting bit position
    int32_t start_bit = bits - 1;
    
    // Ensure valid bit count (limit to 32)
    if (start_bit > 31) start_bit = 31;
    if (start_bit < 0) return;
    
    // Print bits from most to least significant
    for (int32_t i = start_bit; i >= 0; i--) {
        if (value & (1UL << i)) {
            USART_transmit('1');
        } else {
            USART_transmit('0');
        }
        
        // Add space every 8 bits for readability
        if (i % 8 == 0 && i > 0) {
            USART_transmit(' ');
        }
    }
}

uint8_t USART_receive(void) {
    /* Wait until data is available*///datasheet p.210, 219
    while(!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

bool USART_data_available(void) {
    return (UCSR0A & (1 << RXC0));
}