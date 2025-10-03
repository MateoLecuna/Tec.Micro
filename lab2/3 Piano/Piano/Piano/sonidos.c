#include <avr/io.h>
#include "hw_pins.h"
#include "sonidos.h"

#define PRESC_BITS   ((1<<CS22)|(1<<CS20)) // prescaler = 128 en TCCR2B
// TCCR2B: CS22=1, CS21=0, CS20=1 -> 128

// LUT base para una octava: C4..B4 (ver índices en hw_pins.h)
static const uint8_t ocr2a_base[12] = {
	238, // C4
	224, // C#4
	212, // D4
	200, // D#4
	189, // E4
	178, // F4
	168, // F#4
	158, // G4
	149, // G#4
	141, // A4
	133, // A#4
	126  // B4
};

// Ajuste de octava: subir 1 octava -> frecuencia x2 -> OCR pasa ~ a (OCR+1)/2 - 1
static uint8_t ocr_with_octave(uint8_t base, int8_t octave) {
	uint16_t val = (uint16_t)base + 1; // trabajar con (OCR+1)
	if (octave > 0) {
		while (octave--) val = (val > 1) ? (val >> 1) : 1; // divide por 2 con saturación mínima
		} else if (octave < 0) {
		while (octave++) val <<= 1; // multiplica por 2 (baja una octava)
		if (val > 256) val = 256;   // saturar a 256 (OCR<=255)
	}
	uint16_t ocr = (val > 0) ? (val - 1) : 0;
	if (ocr > 255) ocr = 255;
	return (uint8_t)ocr;
}

void sound_init_timer2(void) {
	// Pin OC2A (PB3) como salida ya lo dejaste en hw_init_pins()
	// Timer2 en CTC (WGM21=1), toggle OC2A (COM2A0=1)
	TCCR2A = 0;
	TCCR2B = 0;
	TCNT2  = 0;

	// CTC: WGM21=1 (WGM22=0 en TCCR2B)
	TCCR2A |= (1<<WGM21);
	// Toggle OC2A: COM2A0=1 (no usamos COM2A1)
	TCCR2A |= (1<<COM2A0);

	// Prescaler 128
	TCCR2B |= PRESC_BITS;

	// Arrancamos en silencio (OCR2A arbitrario)
	OCR2A = ocr2a_base[9]; // A4 por defecto, pero OC2A quedará toggling igual
	// Para silencio real, deshabilitamos el toggle hasta que suene algo:
	TCCR2A &= ~(1<<COM2A0); // OC2A desconectado => sin sonido hasta note_on
}

void sound_off(void) {
	// Desconecta OC2A (pin en estado normal de PORTB)
	TCCR2A &= ~(1<<COM2A0);
	// Opcional: forzar pin bajo
	AUDIO_PORT &= ~(1<<AUDIO_PIN);
}

void sound_note_on(uint8_t idx, int8_t octave) {
	if (idx >= 12) { sound_off(); return; }
	uint8_t ocr = ocr_with_octave(ocr2a_base[idx], octave);
	OCR2A = ocr;

	// Asegurar CTC y toggle por si alguien lo tocó
	TCCR2A &= ~((1<<COM2A1)); // COM2A1=0
	TCCR2A |=  (1<<WGM21);    // CTC
	TCCR2B &= ~(1<<WGM22);

	// Habilitar toggle en OC2A (suena)
	TCCR2A |= (1<<COM2A0);
}
