#include "memoria.h"
#include <avr/eeprom.h>
#include <string.h>

char     claveActiva[CLAVE_MAX_LONG + 1];
uint8_t  largoClave = 0;

static uint8_t eeprom_read_u8(uint16_t addr) {
	return eeprom_read_byte((uint8_t*)addr);
}

static void eeprom_write_u8(uint16_t addr, uint8_t v) {
	eeprom_write_byte((uint8_t*)addr, v);
}

static uint8_t eeprom_tiene_clave(void) {
	return eeprom_read_u8(EEPROM_DIR_MAGIA) == EEPROM_VAL_MAGIA;
}

static void eeprom_cargar_clave(void) {
	largoClave = eeprom_read_u8(EEPROM_DIR_LARGO);
	if (largoClave == 0 || largoClave > CLAVE_MAX_LONG) largoClave = 4;
	for (uint8_t i=0; i<largoClave; i++) {
		claveActiva[i] = eeprom_read_u8(EEPROM_DIR_CLAVE + i);
	}
	claveActiva[largoClave] = '\0';
}

void memoria_iniciar(void) {
	if (!eeprom_tiene_clave()) {
		const char* def = "1234";
		uint8_t L = (uint8_t)strlen(def);
		eeprom_write_u8(EEPROM_DIR_MAGIA, EEPROM_VAL_MAGIA);
		eeprom_write_u8(EEPROM_DIR_LARGO, L);
		for (uint8_t i=0; i<L; i++) eeprom_write_u8(EEPROM_DIR_CLAVE + i, (uint8_t)def[i]);
	}
	eeprom_cargar_clave();
}

void memoria_guardar_clave(const char* clave, uint8_t L) {
	if (L == 0 || L > CLAVE_MAX_LONG) return;
	eeprom_write_u8(EEPROM_DIR_MAGIA, EEPROM_VAL_MAGIA);
	eeprom_write_u8(EEPROM_DIR_LARGO, L);
	for (uint8_t i=0; i<L; i++) eeprom_write_u8(EEPROM_DIR_CLAVE + i, (uint8_t)clave[i]);
	// Actualizar RAM
	largoClave = L;
	memcpy(claveActiva, clave, L);
	claveActiva[L] = '\0';
}
