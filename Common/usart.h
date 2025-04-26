#ifndef USART_H
#define USART_H

#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Write a character to USART
 * @param c Character to write
 * @param stream File stream (unused)
 * @return 0 on success
 * 
 * Used by stdio to implement printf/puts. Adds DEBUG: prefix
 * to each new line.
 */
int USART_putchar(char c, FILE *stream);

/**
 * @brief Read a character from USART
 * @param stream File stream (unused)
 * @return Received character
 * 
 * Used by stdio to implement scanf/gets. Blocks until
 * a character is received.
 */
int USART_getchar(FILE *stream);

// Forward declaration for the stream
extern FILE usart_stream;

/**
 * @brief Initialize USART interface
 * @param baudrate Desired communication speed (e.g., 9600, 115200)
 * 
 * Configures the USART hardware module with 8-bit data, 2 stop bits,
 * and specified baud rate. Enables transmitter and receiver.
 * Also redirects stdin/stdout to USART.
 */
void USART_init(uint32_t baudrate);

/**
 * @brief Transmit a single byte
 * @param data Byte to transmit
 * 
 * Blocks until transmit buffer is empty, then sends data
 */
void USART_transmit(uint8_t data);

/**
 * @brief Print a string to USART
 * @param str Zero-terminated string to transmit
 * 
 * Sends each character in the string until null terminator
 */
void USART_print_string(const char* str);

/**
 * @brief Send a byte as binary representation
 * @param data Byte to transmit as binary
 * 
 * Outputs the binary representation of a byte (8 bits)
 * Example: 0b10101010 is output as "10101010"
 */
void USART_send_binary(uint8_t data);

/**
 * @brief Print a 32-bit value as binary
 * @param value The integer value to print in binary
 * @param bits Number of bits to print (max 32)
 * 
 * Outputs bits MSB first, adds spaces between bytes for readability
 * Example: 0x0A0B is output as "00001010 00001011"
 */
void USART_print_binary(uint32_t value, uint8_t bits);

/**
 * @brief Receive a single byte
 * @return Received byte
 * 
 * Blocks until data is available in receive buffer
 */
uint8_t USART_receive(void);

/**
 * @brief Check if data is available
 * @return true if data is available, false otherwise
 */
bool USART_data_available(void);

#endif