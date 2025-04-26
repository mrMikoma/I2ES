#ifndef CHANNEL_H
#define CHANNEL_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize communication channel
 * @param twi_frequency TWI/IC2 frequency in Hz
 * 
 * Initializes TWI hardware in master mode
 */
void channel_init(uint32_t twi_frequency);

/**
 * @brief Initialize communication channel in slave mode
 * @param slave_address 7-bit address to listen on
 * 
 * Initializes TWI hardware in slave mode with the given address
 */
void channel_slave_init(uint8_t slave_address);

/**
 * @brief Send 32-bit integer over TWI
 * @param data Integer to send
 * 
 * Splits integer into 4 bytes and transmits sequentially
 * Includes error checking (non-zero return indicates error)
 */
uint8_t channel_send(int32_t data);

/**
 * @brief Receive 32-bit integer over TWI
 * @return The received 32-bit integer value
 * 
 * Reassembles 4 bytes received sequentially into a 32-bit integer
 * Should be called when data is available (check with channel_available)
 */
int32_t channel_receive(void);

/**
 * @brief Check if data is available to receive
 * @return true if data is available, false otherwise
 * 
 * Checks TWI interface for incoming data
 */
bool channel_available(void);

#endif