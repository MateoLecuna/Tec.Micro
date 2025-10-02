#include <avr/io.h>
#include "hw_pins.h"
#include "sound.h"

void sound_init_idle(void) {
	// Aseguramos pin en bajo y Timer2 detenido
	TCCR2A = 0x00;
	TCCR2B = 0x00;
	OCR2A  = 0;
	AUDIO_PORT &= ~(1<<AUDIO_PIN);
}
