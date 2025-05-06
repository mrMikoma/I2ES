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

// Private helper function prototypes
static void process_received_byte(uint8_t data);

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

uint8_t TWI_get_status(void) {
    // Returns the current TWI status register value
    return TWSR & 0xF8;
}

void TWI_slave_enable(void) {
    // Re-enable slave receiver mode with ACK
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
}

bool TWI_message_available(void) {
    return message_complete;
}

uint32_t TWI_get_last_message(void) {
    message_complete = false;
    return twi_message_buffer;
}

// Process a received byte in the appropriate position of the message
static void process_received_byte(uint8_t data) {
    // Add byte to message buffer in little-endian format
    twi_message_buffer |= ((uint32_t)data << (8 * twi_bytes_received));
    twi_bytes_received++;
    
    if (twi_bytes_received < 3) {
        // Request more bytes with ACK
        TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE) | (1 << TWINT);
    } else if (twi_bytes_received == 3) {
        // For last byte (byte 3), request with NACK
        TWCR = (1 << TWEN) | (1 << TWIE) | (1 << TWINT);
    } else {
        // We've received all 4 bytes
        message_complete = true;
        
        // Call the callback if registered
        if (message_callback != NULL) {
            message_callback(twi_message_buffer);
        }
        
        // Re-enable for next message
        TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE) | (1 << TWINT);
    }
}

// Send a message to the slave
uint8_t TWI_send_message(uint32_t data) {
    uint8_t status;
    
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
        
        // For the last byte (i=3), accept either ACK (0x28) or NACK (0x30)
        if (i == 3) {
            if (status != 0x28 && status != 0x30) {
                printf("Write failed at byte %d: 0x%02X\n", i, status);
                TWI_stop();
                return 0x40 + i; // Error code indicating which byte failed
            }
        } else {
            // For bytes 0-2, require ACK (0x28)
            if (status != 0x28) {
                printf("Write failed at byte %d: 0x%02X\n", i, status);
                TWI_stop();
                return 0x40 + i; // Error code indicating which byte failed
            }
        }
    }
    
    /* Send STOP condition */
    printf("Sending STOP\n");
    TWI_stop();
    
    return 0; // Success
}

// TWI Interrupt Service Routine
ISR(TWI_vect) {
    uint8_t status = TWI_get_status();
    
    // Check if this is an address or data reception status
    if (status == 0x60 || status == 0x68 || status == 0x70 || status == 0x78) {
        // Address received - reset state for new message
        twi_bytes_received = 0;
        twi_message_buffer = 0;
        message_complete = false;
        
        // Prepare to receive first data byte
        TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE) | (1 << TWINT);
    }
    else if (status == 0x80 || status == 0x90) {
        // Data byte received with ACK
        process_received_byte(TWDR);
    }
    else if (status == 0x88 || status == 0x98) {
        // Data received with NACK
        if (twi_bytes_received < 4) {
            process_received_byte(TWDR);
        } else {
            // Re-enable for next message
            TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE) | (1 << TWINT);
        }
    }
    else if (status == 0xA0) {
        // STOP or REPEATED START received
        if (twi_bytes_received == 4) {
            message_complete = true;
            if (message_callback != NULL) {
                message_callback(twi_message_buffer);
            }
        }
        
        // Re-enable for next message
        TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE) | (1 << TWINT);
    }
    else {
        // For any other status, just re-enable
        TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE) | (1 << TWINT);
    }
}