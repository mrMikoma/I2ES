#include <avr/io.h>
#include <stdbool.h>
#include "twi.h"
#include <stdio.h> // For debug prints

void TWI_init_master(uint32_t frequency) {
    // Set SCL frequency using equation from datasheet p.242
    TWBR = ((F_CPU / frequency) - 16) / 2;
    TWSR = 0x00; // Set prescaler to 1 (no prescaling)
    TWCR = (1 << TWEN); // Enable TWI interface
}

void TWI_init_slave(uint8_t address) {
    // Set slave address in TWI address register, shift left by 1 as required by datasheet p.223
    TWAR = (address << 1) | 1; // LSB=1 enables general call recognition
    // Enable TWI interface, enable ACK generation, and clear any pending interrupt flags
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
}

uint8_t TWI_start(uint8_t slave_address) {
    // NOTE: slave_address should be the 7-bit address WITHOUT the R/W bit
    // Step 1 - Send START condition by setting TWINT, TWSTA and TWEN bits (datasheet p.246)
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    // Step 2 - Wait for TWINT flag to be set, indicating START has been transmitted
    while (!(TWCR & (1 << TWINT)));
    
    // Check if START was transmitted successfully
    uint8_t status = TWSR & 0xF8;
    if (status != 0x08 && status != 0x10) {
        return status; // Failed to send START
    }
    
    // Step 3 - Load SLA+W into TWDR register (slave address with write bit)
    // The R/W bit is the LSB - 0 for write, 1 for read
    TWDR = (slave_address << 1) | 0; // 0 = Write operation
    
    // Step 4 - Clear TWINT bit in TWCR to start transmission of address
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    // Step 5 - Wait for TWINT flag to be set, indicating address has been transmitted
    while (!(TWCR & (1 << TWINT)));
    
    // Step 6 - Return status register value to check if slave responded with ACK
    return TWSR & 0xF8;
}

uint8_t TWI_start_read(uint8_t slave_address) {
    // NOTE: slave_address should be the 7-bit address WITHOUT the R/W bit
    // Similar to TWI_start but for read operations
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    while (!(TWCR & (1 << TWINT)));
    
    uint8_t status = TWSR & 0xF8;
    if (status != 0x08 && status != 0x10) {
        return status; // Failed to send START
    }
    
    // Load SLA+R into TWDR register (slave address with read bit)
    TWDR = (slave_address << 1) | 1; // 1 = Read operation
    
    // Clear TWINT bit to start transmission
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    // Wait for completion
    while (!(TWCR & (1 << TWINT)));
    
    // Return status
    return TWSR & 0xF8;
}

uint8_t TWI_write(uint8_t data) {
    // Step 1 - Load data into TWDR register
    TWDR = data;
    
    // Step 2 - Clear TWINT bit in TWCR to start transmission of data
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    // Step 3 - Wait for TWINT flag to be set, indicating data has been transmitted
    while (!(TWCR & (1 << TWINT)));
    
    // Step 4 - Return status register value to check if slave responded with ACK
    return TWSR & 0xF8;
}

void TWI_stop(void) {
    // Transmit STOP condition by setting TWINT, TWSTO and TWEN bits (datasheet p.248)
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
    
    // No need to wait for TWINT here - TWINT is not set after a STOP condition
    // But we can wait until the STOP condition is executed
    while (TWCR & (1 << TWSTO));
}

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
    // For slave mode: Check if TWI is in one of the data reception states
    if (TWCR & (1 << TWINT)) {
        uint8_t status = TWSR & 0xF8;
        
        // Valid slave receiver states per datasheet p.224-226
        switch (status) {
            // Address received states
            case 0x60: // Own SLA+W received, ACK returned
            case 0x68: // Arbitration lost in SLA+R/W, own SLA+W received, ACK returned
            case 0x70: // General call received, ACK returned
            case 0x78: // Arbitration lost in SLA+R/W, general call received, ACK returned
                // printf("Address match detected (0x%02X)\n", status);
                return true;
                
            // Data received states
            case 0x80: // Previously addressed with own SLA+W, data received, ACK returned
            case 0x88: // Previously addressed with own SLA+W, data received, NACK returned
            case 0x90: // Previously addressed with general call, data received, ACK returned
            case 0x98: // Previously addressed with general call, data received, NACK returned
                // printf("Data byte received (0x%02X)\n", status);
                return true;
                
            default:
                break;
        }
    }
    return false;
}

uint8_t TWI_get_status(void) {
    // Returns the current TWI status register value
    return TWSR & 0xF8;
}

void TWI_slave_enable(void) {
    // Re-enable slave receiver mode with ACK
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
}

uint8_t TWI_slave_listen(void) {
    // Wait for an event to occur (TWINT flag set)
    unsigned long timeout = 0;
    while (!(TWCR & (1 << TWINT))) {
        timeout++;
        if (timeout > 100000UL) {
            printf("TWI_slave_listen timeout!\n");
            return 0xFF; // Timeout error
        }
    }
    
    // Return the status code
    uint8_t status = TWSR & 0xF8;
    return status;
}

uint8_t TWI_slave_get_data(void) {
    // Read data from TWDR and acknowledge to receive next byte
    uint8_t data = TWDR;
    
    // Prepare for next byte by sending ACK
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
    
    return data;
}

uint8_t TWI_slave_get_data_nack(void) {
    // Read data from TWDR and send NACK
    uint8_t data = TWDR;
    
    // Prepare for STOP or repeated START by sending NACK
    TWCR = (1 << TWEN) | (1 << TWINT);
    
    return data;
}