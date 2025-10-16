#include "teclado.h"
#include <avr/io.h>
#include <util/delay.h>
#include "hardware.h"

// Mapa de teclas (filas x columnas)
static const char mapa[4][4] = {
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}
};

void teclado_iniciar(void) {
	// Filas PB0..PB3 como salida (inicialmente en 1)
	DDRB |= (1<<TECLADO_FILA0_PB) | (1<<TECLADO_FILA1_PB) | (1<<TECLADO_FILA2_PB) | (1<<TECLADO_FILA3_PB);
	PORTB |= (1<<TECLADO_FILA0_PB) | (1<<TECLADO_FILA1_PB) | (1<<TECLADO_FILA2_PB) | (1<<TECLADO_FILA3_PB);

	// Columnas PB4..PB7 como entrada con pull-up
	DDRB &= ~((1<<TECLADO_COL0_PB)|(1<<TECLADO_COL1_PB)|(1<<TECLADO_COL2_PB)|(1<<TECLADO_COL3_PB));
	PORTB |=  (1<<TECLADO_COL0_PB)|(1<<TECLADO_COL1_PB)|(1<<TECLADO_COL2_PB)|(1<<TECLADO_COL3_PB);
}

static uint8_t leer_columnas(void) {
	uint8_t pin = PINB >> 4; // C0..C3 en bits 0..3
	return pin & 0x0F;
}

char teclado_leer(void) {
	// Escaneo por filas: bajamos una fila a 0, leemos columnas (0=presionada)
	for (uint8_t f=0; f<4; f++) {
		// Poner todas filas en 1
		PORTB |= (1<<TECLADO_FILA0_PB)|(1<<TECLADO_FILA1_PB)|(1<<TECLADO_FILA2_PB)|(1<<TECLADO_FILA3_PB);
		// Bajar fila f
		PORTB &= ~(1<<(TECLADO_FILA0_PB + f));
		_delay_us(15);

		uint8_t cols = leer_columnas(); // 1=libre, 0=presionada
		for (uint8_t c=0; c<4; c++) {
			if (((cols >> c) & 1) == 0) {
				// simple anti-rebote
				_delay_ms(15);
				// esperar a soltar
				while (((leer_columnas() >> c) & 1) == 0);
				return mapa[f][c];
			}
		}
	}
	return '\0';
}
