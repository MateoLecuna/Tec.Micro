#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

/* ==== UART 115200-8N1 con U2X ==== */
#define BAUD 115200
static void uart_init_baud(void) {
	UCSR0A = (1 << U2X0); // doble velocidad
	uint16_t ubrr = (uint16_t)(F_CPU / (8UL * BAUD) - 1UL);
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr & 0xFF);
	UCSR0B = (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8N1
}
static void uart_putc(char c) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}
static void uart_print(const char *s) { while (*s) uart_putc(*s++); }

int main(void) {
	uart_init_baud();
	uart_print("P3C init: UART OK @115200\r\n");
	while (1) {
		uart_print("ping\r\n");
		_delay_ms(500);
	}
}
