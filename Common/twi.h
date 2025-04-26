#ifndef TWI_H
#define TWI_H

#include <stdint.h>
#include <stdbool.h>
#define F_CPU 16000000UL

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
 * @param slave_address 7-bit address to listen on
 * 
 * Configures TWI hardware for slave mode.
 */
void TWI_init_slave(uint8_t slave_address);

/**
 * @brief Send START condition and slave write address
 * @param slave_address 7-bit slave address (without R/W bit)
 * @return TWSR status code (see datasheet p.262)
 * 
 * Sends START condition followed by slave address with write bit (SLA+W)
 */
uint8_t TWI_start(uint8_t slave_address);

/**
 * @brief Send START condition and slave read address
 * @param slave_address 7-bit slave address (without R/W bit)
 * @return TWSR status code (see datasheet p.262)
 * 
 * Sends START condition followed by slave address with read bit (SLA+R)
 */
uint8_t TWI_start_read(uint8_t slave_address);

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

#endif