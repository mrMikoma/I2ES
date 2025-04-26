#include <avr/io.h>
#include <stdbool.h>
#include "twi.h"

void TWI_init_master(uint32_t frequency) {
    // Set SCL frequency using equation from datasheet p.242
    TWBR = ((F_CPU / frequency) - 16) / 2;
    TWSR = 0x00; // Set prescaler to 1 (no prescaling)
    TWCR = (1 << TWEN); // Enable TWI interface
}

void TWI_init_slave(uint8_t address) {
    // Set slave address in TWI address register, shift left by 1 as required by datasheet p.223
    TWAR = (address << 1);
    // Enable TWI interface and acknowledge bit
    TWCR = (1 << TWEN) | (1 << TWEA);
}

uint8_t TWI_start(uint8_t slave_address) {
    // Step 1 - Send START condition by setting TWINT, TWSTA and TWEN bits (datasheet p.246)
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    // Step 2 - Wait for TWINT flag to be set, indicating START has been transmitted
    while (!(TWCR & (1 << TWINT)));
    
    // Step 3 - Load SLA+W/R into TWDR register (slave address with write/read bit)
    TWDR = slave_address;
    
    // Step 4 - Clear TWINT bit in TWCR to start transmission of address
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    // Step 5 - Wait for TWINT flag to be set, indicating address has been transmitted
    while (!(TWCR & (1 << TWINT)));
    
    // Step 6 - Return status register value masked with 0xF8 to ignore prescaler bits (datasheet p.262)
    return TWSR & 0xF8;
}

uint8_t TWI_write(uint8_t data) {
    // Step 1 - Load data into TWDR register
    TWDR = data;
    
    // Step 2 - Clear TWINT bit in TWCR to start transmission of data
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    // Step 3 - Wait for TWINT flag to be set, indicating data has been transmitted
    while (!(TWCR & (1 << TWINT)));
    
    // Step 4 - Return status register value masked with 0xF8 (datasheet p.262)
    return TWSR & 0xF8;
}

void TWI_stop(void) {
    // Transmit STOP condition by setting TWINT, TWSTO and TWEN bits (datasheet p.248)
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

// Add the missing functions
uint8_t TWI_read_ack(void) {
    // Set TWINT, TWEN and TWEA bits to receive byte and respond with ACK
    // TWEA bit enables ACK pulse generation after byte is received (datasheet p.249)
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    
    // Wait for TWINT flag to be set, indicating byte has been received
    while (!(TWCR & (1 << TWINT)));
    
    // Return received data from TWDR
    return TWDR;
}

uint8_t TWI_read_nack(void) {
    // Set TWINT and TWEN without TWEA to receive byte and respond with NACK
    // Not setting TWEA means generate NACK after byte is received (datasheet p.249)
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    // Wait for TWINT flag to be set, indicating byte has been received
    while (!(TWCR & (1 << TWINT)));
    
    // Return received data from TWDR
    return TWDR;
}

bool TWI_data_available(void) {
    // Check if TWINT bit is set indicating TWI interface has completed operation
    // Returns true if data available, false otherwise
    return (TWCR & (1 << TWINT)) != 0;
}