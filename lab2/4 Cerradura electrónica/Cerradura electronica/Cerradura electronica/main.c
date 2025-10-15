#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#include "hardware.h"
#include "memoria.h"
#include "logica_candado.h"

int main(void) {
	hardware_iniciar();    // clock, LEDs, TWI, LCD
	memoria_iniciar();     // carga "1234" si no existe
	candado_iniciar();     // muestra "BLOQ / Ingrese clave"

	for (;;) {
		candado_loop();    // bucle principal de la cerradura
	}
}
