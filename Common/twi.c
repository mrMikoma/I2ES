#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include "twi.h"
#include <stdio.h> // For debug prints

// Global variables for interrupt-based message reception
static volatile uint32_t twi_message_buffer = 0;
static volatile uint8_t twi_bytes_received = 0;
static twi_message_callback_t message_callback = NULL;
static volatile bool message_complete = false;

void TWI_set_callback(twi_message_callback_t callback) {
    message_callback = callback;
}

void TWI_init_master(uint32_t frequency) {
    // Set SCL frequency using equation from datasheet p.242
    TWBR = ((F_CPU / frequency) - 16) / 2;
    TWSR = 0x00; // Set prescaler to 1 (no prescaling)
    TWCR = (1 << TWEN); // Enable TWI interface
}

void TWI_init_slave(void) {
    // Set slave address in TWI address register, shift left by 1 as required by datasheet p.223
    TWAR = (SLAVE_ADDRESS << 1) | 1; // LSB=1 enables general call recognition
    // Enable TWI interface, enable ACK generation, and clear any pending interrupt flags
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
    
    // Reset message state
    twi_bytes_received = 0;
    message_complete = false;
}

void TWI_enable_interrupt(bool enable) {
    // Reset message buffer and byte counter
    twi_bytes_received = 0;
    twi_message_buffer = 0;
    message_complete = false;
    
    if (enable) {
        // Enable TWI interrupt
        TWCR |= (1 << TWIE);
        // Enable global interrupts
        sei();
    } else {
        // Disable TWI interrupt
        TWCR &= ~(1 << TWIE);
    }
}

uint8_t TWI_start(void) {
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
    TWDR = (SLAVE_ADDRESS << 1) | 0; // 0 = Write operation
    
    // Step 4 - Clear TWINT bit in TWCR to start transmission of address
    TWCR = (1 << TWINT) | (1 << TWEN);
    
    // Step 5 - Wait for TWINT flag to be set, indicating address has been transmitted
    while (!(TWCR & (1 << TWINT)));
    
    // Step 6 - Return status register value to check if slave responded with ACK
    return TWSR & 0xF8;
}

uint8_t TWI_start_read(void) {
    // Similar to TWI_start but for read operations
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    
    while (!(TWCR & (1 << TWINT)));
    
    uint8_t status = TWSR & 0xF8;
    if (status != 0x08 && status != 0x10) {
        return status; // Failed to send START
    }
    
    // Load SLA+R into TWDR register (slave address with read bit)
    TWDR = (SLAVE_ADDRESS << 1) | 1; // 1 = Read operation
    
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
        if (timeout > 100000UL) { // 100ms timeout, if cpu is running at 16MHz
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

bool TWI_message_available(void) {
    return message_complete;
}

uint32_t TWI_get_last_message(void) {
    message_complete = false;
    return twi_message_buffer;
}

// Blocking function to receive a complete message
uint32_t TWI_receive_message(void) {
    uint32_t data = 0;
    
    // If we already have a complete message, return it
    if (message_complete) {
        message_complete = false;
        return twi_message_buffer;
    }
    
    // disable interrupts
    cli();

    // Get current TWI status
    uint8_t status = TWI_get_status();
    
    // If we're at address match state, prepare for data reception
    if (status == 0x60 || status == 0x68 || status == 0x70 || status == 0x78) {
        // Clear interrupt flag and enable ACK to receive first data byte
        TWI_slave_enable();
        
        // Wait for first data byte
        status = TWI_slave_listen();
        
        if (status != 0x80 && status != 0x90) {
            // Something went wrong, re-enable slave receiver
            TWI_slave_enable();
            return 0;
        }
    }
    
    // We should now be at data received state (0x80 or 0x90)
    // Read 4 bytes (32-bit integer) in little-endian format
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t byte;
        
        // For bytes 0-2, read with ACK
        // For byte 3 (last), read with NACK
        if (i < 3) {
            byte = TWI_slave_get_data();
            
            // Check if we're still receiving data properly
            status = TWI_slave_listen();
            
            if (status != 0x80 && status != 0x90) {
                // Lost connection or error
                TWI_slave_enable();
                return data; // Return partial data
            }
        } else {
            // Last byte - get with NACK
            byte = TWI_slave_get_data_nack();
        }
        
        // Add byte to result (little-endian)
        data |= ((uint32_t)byte << (8*i));
    }
    
    // Re-enable slave receiver mode for next message
    TWI_slave_enable();

    // enable interrupts
    sei();
    
    return data;
}

// Send a message to the slave
uint8_t TWI_send_message(uint32_t data) {
    uint8_t status;
    
    // disable interrupts
    cli();

    /* Send START condition and SLA+W */
    printf("Sending START + address 0x%02X\n", SLAVE_ADDRESS);
    status = TWI_start();
    if (status != 0x18) { // SLA+W sent, ACK received
        printf("START failed: 0x%02X\n", status);
        TWI_stop();
        return status; // Return error code
    }
    
    /* Send data bytes (little-endian order) */
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t byte = (data >> (8*i)) & 0xFF;
        printf("Sending byte %d: 0x%02X\n", i, byte);
        status = TWI_write(byte);
        if (status != 0x28) { // Data byte sent, ACK received
            printf("Write failed at byte %d: 0x%02X\n", i, status);
            TWI_stop();
            return 0x40 + i; // Error code indicating which byte failed
        }
    }
    
    /* Send STOP condition */
    printf("Sending STOP\n");
    TWI_stop();

    // enable interrupts
    sei();
    
    return 0; // Success
}

// TWI Interrupt Service Routine
ISR(TWI_vect) {
    // disable interrupts 
    cli();
    uint32_t message = TWI_receive_message();
    if (message_callback) {
        message_callback(message);
    }
    // enable interrupts 
    sei();
}