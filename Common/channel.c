#include "channel.h"
#include "twi.h"
#include "usart.h" // Add for printf debugging
#include <stdio.h>

#define SLAVE_ADDR 0x57

// Uses TWI functions for all I2C communication
// This file implements higher-level protocol for sending/receiving 32-bit messages

void channel_init(uint32_t twi_frequency) {
    // Initialize TWI master with the specified frequency
    TWI_init_master(twi_frequency);
    printf("TWI Master initialized at %lu Hz\n", twi_frequency);
}

void channel_slave_init(uint8_t slave_address) {
    // Initialize TWI slave with the specified address
    printf("Initializing TWI slave with address 0x%02X\n", slave_address);
    
    // Enable general call recognition for more reliable detection
    TWI_init_slave(slave_address);
    
    // Enable slave mode and report initial status
    uint8_t status = TWI_get_status();
    printf("TWI Slave initialized. Initial status: 0x%02X\n", status);
}

uint8_t channel_send(int32_t data) {
    uint8_t status;
    
    /* Send START condition and SLA+W */
    printf("Sending START + address 0x%02X\n", SLAVE_ADDR);
    status = TWI_start(SLAVE_ADDR);
    if (status != 0x18) { // SLA+W sent, ACK received
        printf("START failed: 0x%02X\n", status);
        TWI_stop();
        return status; // Return error code
    }
    
    /* Send data bytes (little-endian order) */
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t byte = (data >> (8*i)) & 0xFF;
       // printf("Sending byte %d: 0x%02X\n", i, byte);
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
    return 0; // Success
}

bool channel_available(void) {
    // Check if there's data available on the TWI bus
    bool available = TWI_data_available();
    if (available) {
        // Only print when data becomes available
        static bool was_available = false;
        if (!was_available) {
            printf("TWI data is now available (status: 0x%02X)\n", TWI_get_status());
            was_available = true;
        }
    } else {
        // Reset was_available when data is no longer available
        static bool was_available = true;
        if (was_available) {
            was_available = false;
        }
    }
    return available;
}

int32_t channel_receive(void) {
    int32_t data = 0;
    
    //printf("Starting channel_receive\n");
    
    // Get current TWI status
    uint8_t status = TWI_get_status();
    //printf("Initial status: 0x%02X\n", status);
    
    // If we're at address match state, prepare for data reception
    if (status == 0x60 || status == 0x68 || status == 0x70 || status == 0x78) {
        //printf("Address match detected\n");
        
        // Clear interrupt flag and enable ACK to receive first data byte
        TWI_slave_enable();
        //printf("Slave enabled, waiting for data\n");
        
        // Wait for first data byte
        status = TWI_slave_listen();
        //printf("After listen, status: 0x%02X\n", status);
        
        if (status != 0x80 && status != 0x90) {
            // Something went wrong, re-enable slave receiver
            printf("Error: Expected data received status, got 0x%02X\n", status);
            TWI_slave_enable();
            return 0;
        }
    }
    
    // We should now be at data received state (0x80 or 0x90)
    //printf("Reading 4 bytes of data\n");
    
    // Read 4 bytes (32-bit integer) in little-endian format
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t byte;
        
        // For bytes 0-2, read with ACK
        // For byte 3 (last), read with NACK
        if (i < 3) {
            //printf("Reading byte %d with ACK\n", i);
            byte = TWI_slave_get_data();
            
            // Check if we're still receiving data properly
            status = TWI_slave_listen();
            //printf("After byte %d, status: 0x%02X\n", i, status);
            
            if (status != 0x80 && status != 0x90) {
                // Lost connection or error
                printf("Error after byte %d: status 0x%02X\n", i, status);
                TWI_slave_enable();
                return data; // Return partial data
            }
        } else {
            // Last byte - get with NACK
            //printf("Reading final byte with NACK\n");
            byte = TWI_slave_get_data_nack();
        }
        
        //printf("Received byte %d: 0x%02X\n", i, byte);
        
        // Add byte to result (little-endian)
        data |= ((int32_t)byte << (8*i));
    }
    
    printf("Message reception complete: 0x%08lX\n", data);
    
    // Re-enable slave receiver mode for next message
    TWI_slave_enable();
    
    return data;
}