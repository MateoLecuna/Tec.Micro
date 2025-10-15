#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "piano.h"
#include "sonidos.h"

#define DEBOUNCE_MS 8
static int8_t s_octave = 0;

// Mapeo de teclas
// A0..A5 = PC0..PC5 --> C..F (índices 0–5)
#define K00_PINREG  PINC
#define K00_DDR     DDRC
#define K00_PORT    PORTC
#define K00_BIT     0   // C
#define K01_PINREG  PINC
#define K01_DDR     DDRC
#define K01_PORT    PORTC
#define K01_BIT     1   // C#
#define K02_PINREG  PINC
#define K02_DDR     DDRC
#define K02_PORT    PORTC
#define K02_BIT     2   // D
#define K03_PINREG  PINC
#define K03_DDR     DDRC
#define K03_PORT    PORTC
#define K03_BIT     3   // D#
#define K04_PINREG  PINC
#define K04_DDR     DDRC
#define K04_PORT    PORTC
#define K04_BIT     4   // E
#define K05_PINREG  PINC
#define K05_DDR     DDRC
#define K05_PORT    PORTC
#define K05_BIT     5   // F

// D2..D7 = PD2..PD7 --> F#..B (índices 6–11)
#define K06_PINREG  PIND
#define K06_DDR     DDRD
#define K06_PORT    PORTD
#define K06_BIT     2   // F#
#define K07_PINREG  PIND
#define K07_DDR     DDRD
#define K07_PORT    PORTD
#define K07_BIT     3   // G
#define K08_PINREG  PIND
#define K08_DDR     DDRD
#define K08_PORT    PORTD
#define K08_BIT     4   // G#
#define K09_PINREG  PIND
#define K09_DDR     DDRD
#define K09_PORT    PORTD
#define K09_BIT     5   // A
#define K10_PINREG  PIND
#define K10_DDR     DDRD
#define K10_PORT    PORTD
#define K10_BIT     6   // A#
#define K11_PINREG  PIND
#define K11_DDR     DDRD
#define K11_PORT    PORTD
#define K11_BIT     7   // B

// Funciones internas
static inline uint8_t key_raw(uint8_t k) {
	switch (k) {
		case  0: return (K00_PINREG & (1<<K00_BIT)) ? 1:0;
		case  1: return (K01_PINREG & (1<<K01_BIT)) ? 1:0;
		case  2: return (K02_PINREG & (1<<K02_BIT)) ? 1:0;
		case  3: return (K03_PINREG & (1<<K03_BIT)) ? 1:0;
		case  4: return (K04_PINREG & (1<<K04_BIT)) ? 1:0;
		case  5: return (K05_PINREG & (1<<K05_BIT)) ? 1:0;
		case  6: return (K06_PINREG & (1<<K06_BIT)) ? 1:0;
		case  7: return (K07_PINREG & (1<<K07_BIT)) ? 1:0;
		case  8: return (K08_PINREG & (1<<K08_BIT)) ? 1:0;
		case  9: return (K09_PINREG & (1<<K09_BIT)) ? 1:0;
		case 10: return (K10_PINREG & (1<<K10_BIT)) ? 1:0;
		case 11: return (K11_PINREG & (1<<K11_BIT)) ? 1:0;
	}
	return 1;
}


void piano_init(void) {
	// Entradas + pull-ups
	K00_DDR &= ~(1<<K00_BIT); K00_PORT |= (1<<K00_BIT);
	K01_DDR &= ~(1<<K01_BIT); K01_PORT |= (1<<K01_BIT);
	K02_DDR &= ~(1<<K02_BIT); K02_PORT |= (1<<K02_BIT);
	K03_DDR &= ~(1<<K03_BIT); K03_PORT |= (1<<K03_BIT);
	K04_DDR &= ~(1<<K04_BIT); K04_PORT |= (1<<K04_BIT);
	K05_DDR &= ~(1<<K05_BIT); K05_PORT |= (1<<K05_BIT);
	K06_DDR &= ~(1<<K06_BIT); K06_PORT |= (1<<K06_BIT);
	K07_DDR &= ~(1<<K07_BIT); K07_PORT |= (1<<K07_BIT);
	K08_DDR &= ~(1<<K08_BIT); K08_PORT |= (1<<K08_BIT);
	K09_DDR &= ~(1<<K09_BIT); K09_PORT |= (1<<K09_BIT);
	K10_DDR &= ~(1<<K10_BIT); K10_PORT |= (1<<K10_BIT);
	K11_DDR &= ~(1<<K11_BIT); K11_PORT |= (1<<K11_BIT);
}

void piano_mode_tick(void) {
	int8_t pressed = -1;

	for (uint8_t k = 0; k < 12; ++k) {
		if (key_raw(k) == 0) { // tecla presionada (activo en LOW)
			_delay_ms(DEBOUNCE_MS);
			if (key_raw(k) == 0) { pressed = (int8_t)k; break; }
		}
	}

	if (pressed >= 0)
	sound_note_on((uint8_t)pressed, s_octave);
	else
	sound_off();
}
