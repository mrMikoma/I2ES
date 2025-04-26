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
 * @brief Send START condition and slave address
 * @param slave_address 7-bit slave address with R/W bit cleared
 * @return TWSR status code (see datasheet p.262)
 * 
 * Follows sequence from datasheet p.246
 */
uint8_t TWI_start(uint8_t slave_address);

/**
 * @brief Write data byte to TWI bus
 * @param data Byte to transmit
 * @return TWSR status code (see datasheet p.262)
 */
uint8_t TWI_write(uint8_t data);

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
 */
bool TWI_data_available(void);

/**
 * @brief Send STOP condition
 * 
 * Releases TWI bus (see datasheet p.248)
 */
void TWI_stop(void);

#endif