#include "channel.h"
#include "twi.h"

#define SLAVE_ADDR 0x57 // Example slave address

void channel_init(uint32_t twi_frequency) {
    TWI_init_master(twi_frequency);
}

uint8_t channel_send(int32_t data) {
    uint8_t status;
    
    /* Start transmission (p.246 step 1-4) */
    if((status = TWI_start(SLAVE_ADDR)) != 0x18) {
        TWI_stop();
        return status; // Return error code
    }
    
    /* Send data bytes (p.246 step 5b-6) */
    for(uint8_t i = 0; i < 4; i++) {
        uint8_t byte = (data >> (8*i)) & 0xFF;
        if((status = TWI_write(byte)) != 0x28) {
            TWI_stop();
            return status + i; // Encode byte position in error
        }
    }
    
    TWI_stop();
    return 0; // Success
}

void channel_slave_init(uint8_t slave_address) {
    TWI_init_slave(slave_address);
}

bool channel_available(void) {
    return TWI_data_available();
}

int32_t channel_receive(void) {
    int32_t data = 0;
    
    // Wait for first data byte (address match already occurred)
    while(!TWI_data_available()) {;}
    
    // Read 4 bytes in little-endian format (LSB first)
    for(uint8_t i = 0; i < 4; i++) {
        uint8_t byte;
        // Read with ACK for first 3 bytes, NACK for last byte
        if(i < 3) {
            byte = TWI_read_ack();
        } else {
            byte = TWI_read_nack();
        }
        
        // Assemble integer (little-endian)
        data |= ((int32_t)byte << (8*i));
    }
    
    return data;
}