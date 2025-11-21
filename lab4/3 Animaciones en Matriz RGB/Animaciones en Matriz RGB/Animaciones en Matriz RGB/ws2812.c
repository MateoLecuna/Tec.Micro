#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>

#include "ws2812.h"

// Máscara del pin
#define WS2812_BIT (1 << WS2812_PIN)

// Tiempos aproximados (us)
// Datos de WS2812 (aprox):
//  '0' -> T_high ~0.35us, T_low ~0.8us
//  '1' -> T_high ~0.7us,  T_low ~0.6us
// Como además hay tiempo de las instrucciones, usamos valores un poco más separados.

static inline void ws2812_send_bit(uint8_t bit_val) {
	if (bit_val) {
		// '1' -> HIGH más largo
		WS2812_PORT |= WS2812_BIT;      // HIGH
		_delay_us(0.7);                 // alta larga
		WS2812_PORT &= ~WS2812_BIT;     // LOW
		_delay_us(0.6);
		} else {
		// '0' -> HIGH más corto
		WS2812_PORT |= WS2812_BIT;      // HIGH
		_delay_us(0.2);                 // alta muy cortita
		WS2812_PORT &= ~WS2812_BIT;     // LOW
		_delay_us(1.0);                 // baja más larga
	}
}

static void ws2812_send_byte(uint8_t b) {
	for (int8_t i = 7; i >= 0; i--) {
		ws2812_send_bit((b >> i) & 0x01);
	}
}

void ws2812_init(void) {
	WS2812_DDR |= WS2812_BIT;   // pin como salida
	WS2812_PORT &= ~WS2812_BIT; // empezar en LOW
}

void ws2812_show(uint8_t *fb) {
	// Deshabilitar interrupciones para no romper tiempos
	uint8_t sreg = SREG;
	cli();

	// 64 LEDs * 3 bytes (G, R, B)
	for (uint16_t i = 0; i < 64 * 3; i++) {
		ws2812_send_byte(fb[i]);
	}

	// Reset/latch > 50us
	_delay_us(80);

	// Restaurar interrupciones
	SREG = sreg;
}
