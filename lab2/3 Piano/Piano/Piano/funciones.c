#define F_CPU 16000000UL

#include <util/delay.h>
#include <stdint.h>

#include "funciones.h"
#include "sonidos.h"

void delay_ms_var(uint16_t ms) {
	while (ms--) _delay_ms(1);
}

void play_note_ms(uint8_t idx, int8_t octave, uint16_t ms) {
	sound_note_on(idx, octave);
	while (ms--) _delay_ms(1);
	sound_off();
	_delay_ms(10);
}