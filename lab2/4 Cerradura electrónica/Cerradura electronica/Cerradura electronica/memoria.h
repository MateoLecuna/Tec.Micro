#ifndef MEMORIA_H
#define MEMORIA_H
#include <stdint.h>

#define CLAVE_MAX_LONG   16
#define EEPROM_DIR_MAGIA 0
#define EEPROM_VAL_MAGIA 0xA5
#define EEPROM_DIR_LARGO 1
#define EEPROM_DIR_CLAVE 2

extern char claveActiva[CLAVE_MAX_LONG + 1];
extern uint8_t largoClave;

void memoria_iniciar(void);
void memoria_guardar_clave(const char* clave, uint8_t L);

#endif
