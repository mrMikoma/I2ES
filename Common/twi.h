#ifndef TWI_H
#define TWI_H

#include <stdint.h>
#include <stdbool.h>
#define F_CPU 16000000UL

// Default slave address for TWI communication
#define SLAVE_ADDRESS 0x57

// Message handling callback type definition
typedef void (*twi_message_callback_t)(uint32_t message);

/**
 * @brief Set callback function for message reception
 * @param callback Function to call when a message is received
 */
void TWI_set_callback(twi_message_callback_t callback);

/**
 * @brief Initialize TWI in master mode
 * @param frequency Desired SCL frequency in Hz
 * 
 * Configures TWI hardware with specified frequency.
 * Prescaler set to 1 (see datasheet p.242 for calculation)
 */
void TWI_init_master(uint32_t frequency);

/**
 * @brief Initialize TWI in slave mode
 * 
 * Configures TWI hardware for slave mode with SLAVE_ADDRESS.
 */
void TWI_init_slave(void);

/**
 * @brief Send START condition and slave write address
 * @return TWSR status code (see datasheet p.262)
 * 
 * Sends START condition followed by SLAVE_ADDRESS with write bit (SLA+W)
 */
uint8_t TWI_start(void);

/**
 * @brief Send START condition and slave read address
 * @return TWSR status code (see datasheet p.262)
 * 
 * Sends START condition followed by SLAVE_ADDRESS with read bit (SLA+R)
 */
uint8_t TWI_start_read(void);

/**
 * @brief Write data byte to TWI bus
 * @param data Byte to transmit
 * @return TWSR status code (see datasheet p.262)
 */
uint8_t TWI_write(uint8_t data);

/**
 * @brief Send STOP condition
 * 
 * Releases TWI bus (see datasheet p.248)
 */
void TWI_stop(void);

/**
 * @brief Read data byte from TWI bus with ACK
 * @return Received byte
 * 
 * Sends ACK after receiving byte (expects more data)
 */
uint8_t TWI_read_ack(void);

/**
 * @brief Read data byte from TWI bus with NACK
 * @return Received byte
 * 
 * Sends NACK after receiving byte (last byte to read)
 */
uint8_t TWI_read_nack(void);

/**
 * @brief Check if data is available for slave
 * @return true if data is available, false otherwise
 * 
 * Checks TWI status register for slave receive states
 */
bool TWI_data_available(void);

/**
 * @brief Get current TWI status register value
 * @return Status code (masked with 0xF8)
 */
uint8_t TWI_get_status(void);

/**
 * @brief Re-enable slave receiver mode
 * 
 * Call after processing a message or to recover from errors
 */
void TWI_slave_enable(void);

/**
 * @brief Wait for TWI slave event
 * @return Status code indicating event type
 * 
 * Blocks until TWI interrupt flag is set
 */
uint8_t TWI_slave_listen(void);

/**
 * @brief Read received data and acknowledge
 * @return Received data byte
 * 
 * Use when expecting more data bytes
 */
uint8_t TWI_slave_get_data(void);

/**
 * @brief Read received data and send NACK
 * @return Received data byte
 * 
 * Use for last byte of message
 */
uint8_t TWI_slave_get_data_nack(void);

/**
 * @brief Enable or disable interrupt-driven message reception
 * @param enable true to enable interrupts, false to disable
 *
 * When enabled, received messages will trigger the callback function
 */
void TWI_enable_interrupt(bool enable);

/**
 * @brief Get a complete 32-bit message
 * @return 32-bit received message
 *
 * This function blocks until a complete message is received
 */
uint32_t TWI_receive_message(void);

/**
 * @brief Check if a complete message is available
 * @return true if a complete message is available, false otherwise
 */
bool TWI_message_available(void);

/**
 * @brief Get the last received message
 * @return The 32-bit message value
 */
uint32_t TWI_get_last_message(void);

/**
 * @brief Send a 32-bit message to the slave address
 * @param data The 32-bit message to send
 * @return 0 on success, error code otherwise
 * 
 * Sends a 32-bit integer broken down into 4 bytes in little-endian format
 * over the TWI bus to the SLAVE_ADDRESS.
 */
uint8_t TWI_send_message(uint32_t data);

#endif