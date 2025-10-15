#include "twi.h"
#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

void twi_init(uint32_t scl_hz) {
	// Prescaler = 1
	TWSR = 0x00;
	// TWBR formula: SCL = F_CPU / (16 + 2*TWBR*Prescaler)
	uint32_t twbr_val = ((F_CPU / scl_hz) - 16) / 2;
	if (twbr_val > 255) twbr_val = 255;
	TWBR = (uint8_t)twbr_val;
	TWCR = (1<<TWEN);
}

static uint8_t twi_status(void) {
	return TWSR & 0xF8;
}

uint8_t twi_start(uint8_t address_rw) {
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	uint8_t st = twi_status();
	if (st != 0x08 && st != 0x10) return 0; // START o REPEATED START

	TWDR = address_rw;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	st = twi_status();
	if (st != 0x18 && st != 0x40) return 0; // SLA+W ACK o SLA+R ACK
	return 1;
}

void twi_stop(void) {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

uint8_t twi_write(uint8_t data) {
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	return (twi_status() == 0x28); // DATA ACK
}

uint8_t twi_read_ack(uint8_t* data) {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while (!(TWCR & (1<<TWINT)));
	*data = TWDR;
	return 1;
}

uint8_t twi_read_nack(uint8_t* data) {
	TWCR = (1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	*data = TWDR;
	return 1;
}
