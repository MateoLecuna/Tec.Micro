#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "hw_pins.h"
#include "sonidos.h"
#include "figuras.h"
#include "canciones.h"
#include "funciones.h"

int main(void) {
	hw_init_pins();
	sound_init_timer2();

	while (1) {
		c_asecina();
	}
}
