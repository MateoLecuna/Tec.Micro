#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include "UART.h"
#include "animaciones.h"
#include "ws2812.h"

// Configuración UART
#define BAUD 9600UL
#define UBRR_VAL ((F_CPU / (16UL * BAUD)) - 1)

// Buffer de la matriz: 64 LEDs * 3 bytes (G, R, B)
static uint8_t frame_buffer[64 * 3];

// Tick en milisegundos (lo incrementa el Timer0)
volatile uint32_t tick_ms = 0;

// Prototipos internos
static void timer0_init(void);
static uint8_t uart_try_receive(char *c);
static void procesar_char(char c);

// Pequeño parser para comandos "1", "2", "0" y "OFF"
static char cmd_buf[4] = {0};
static uint8_t cmd_index = 0;

// ISR Timer0 CTC, cada 1 ms
ISR(TIMER0_COMPA_vect) {
	tick_ms++;
}

// Inicialización de Timer0 en modo CTC para generar tick_ms
static void timer0_init(void) {
	// Modo CTC
	TCCR0A = (1 << WGM01);
	// Prescaler 64
	TCCR0B = (1 << CS01) | (1 << CS00);
	// F_CPU = 16 MHz ? 16000000 / (64 * 1000) = 250 ? OCR0A = 249
	OCR0A = 249;
	// Habilitar interrupción de comparación A
	TIMSK0 = (1 << OCIE0A);
}

// Lectura UART no bloqueante
static uint8_t uart_try_receive(char *c) {
	if (UCSR0A & (1 << RXC0)) {
		*c = UDR0;
		return 1;
	}
	return 0;
}

// Procesa caracteres recibidos por UART y cambia de modo
static void procesar_char(char c) {
	// Ignorar CR / LF
	if (c == '\r' || c == '\n') {
		cmd_index = 0;
		cmd_buf[0] = cmd_buf[1] = cmd_buf[2] = '\0';
		return;
	}

	// Comandos de un solo carácter
	if (c == '1' || c == '2' || c == '0') {
		if (c == '1') {
			animaciones_set_modo(MODO_ANIM1, frame_buffer);
			uart_print("MODO 1\r\n");
			} else if (c == '2') {
			animaciones_set_modo(MODO_ANIM2, frame_buffer);
			uart_print("MODO 2\r\n");
			} else {
			animaciones_set_modo(MODO_OFF, frame_buffer);
			uart_print("MODO OFF\r\n");
		}

		cmd_index = 0;
		cmd_buf[0] = cmd_buf[1] = cmd_buf[2] = '\0';
		return;
	}

	// Acumulamos para detectar la palabra "OFF"
	if (cmd_index < 3) {
		cmd_buf[cmd_index++] = c;
		cmd_buf[cmd_index] = '\0';
		} else {
		cmd_buf[0] = cmd_buf[1];
		cmd_buf[1] = cmd_buf[2];
		cmd_buf[2] = c;
	}

	if (cmd_buf[0] == 'O' && cmd_buf[1] == 'F' && cmd_buf[2] == 'F') {
		animaciones_set_modo(MODO_OFF, frame_buffer);
		uart_print("MODO OFF\r\n");

		cmd_index = 0;
		cmd_buf[0] = cmd_buf[1] = cmd_buf[2] = '\0';
	}
}

int main(void) {
	// Inicializar UART
	uart_init(UBRR_VAL);
	uart_print("Sistema iniciado\r\n");

	// Inicializar Timer0 para tick_ms
	timer0_init();

	// Inicializar módulo de animaciones
	animaciones_init(frame_buffer);

	// Inicializar driver de WS2812
	ws2812_init();

	// Habilitar interrupciones globales
	sei();

	while (1) {
		// Leer UART sin bloquear
		char c;
		if (uart_try_receive(&c)) {
			procesar_char(c);
		}

		// Actualizar animaciones de forma no bloqueante
		uint32_t t = tick_ms;
		if (animaciones_update(t, frame_buffer)) {
			// Si hubo actualización del frame, se envía a la matriz
			ws2812_show(frame_buffer);
		}
	}

	return 0;
}
