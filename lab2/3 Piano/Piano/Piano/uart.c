#include "uart.h"
#include <avr/interrupt.h>

#define RX_BUF_SZ 64
static volatile uint8_t rx_buf[RX_BUF_SZ];
static volatile uint8_t rx_head = 0, rx_tail = 0;

void uart_init(uint32_t baud) {
	// 8N1, sin U2X (error ~0.2% a 9600)
	uint16_t ubrr = (F_CPU / (16UL * baud)) - 1;
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr & 0xFF);

	UCSR0A = 0;                                // U2X0=0
	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);  // RX/TX + RX interrupt
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);        // 8 data, 1 stop, no parity

	sei();
}

void uart_putchar(char c) {
	if (c == '\n') uart_putchar('\r');         // CRLF para monitores típicos
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = c;
}

void uart_write(const char *s) {
	while (*s) uart_putchar(*s++);
}

uint8_t uart_available(void) {
	return (uint8_t)((RX_BUF_SZ + rx_head - rx_tail) % RX_BUF_SZ);
}

int16_t uart_getchar(void) {
	if (rx_head == rx_tail) return -1;
	uint8_t c = rx_buf[rx_tail];
	rx_tail = (rx_tail + 1) % RX_BUF_SZ;
	return c;
}

// Lee hasta '\n' o '\r'
int8_t uart_readline(char *dst, uint8_t maxlen) {
	if (maxlen == 0) return 0;
	uint8_t n = 0;
	int16_t ch;
	// No bloquear: solo arma línea si ya llegó el terminador
	// 1) Mirar si hay '\n' en el buffer
	uint8_t i = rx_tail;
	uint8_t have_eol = 0;
	while (i != rx_head) {
		uint8_t c = rx_buf[i];
		if (c == '\n' || c == '\r') { have_eol = 1; break; }
		i = (i + 1) % RX_BUF_SZ;
	}
	if (!have_eol) return 0;

	// 2) Extraer hasta EOL
	while ((ch = uart_getchar()) != -1) {
		char c = (char)ch;
		if (c == '\n' || c == '\r') break;
		if (n < (uint8_t)(maxlen - 1)) dst[n++] = c;   // evitar overflow
	}
	dst[n] = '\0';
	// "Comer" duplicado si vino \r\n
	int16_t peek = uart_getchar();
	if (peek != -1) {
		char p = (char)peek;
		if (!(p == '\n' || p == '\r')) {
		}
	}
	return (int8_t)n;
}

ISR(USART_RX_vect) {
	uint8_t d = UDR0;
	uint8_t next = (rx_head + 1) % RX_BUF_SZ;
	if (next != rx_tail) {                 // si hay espacio
		rx_buf[rx_head] = d;
		rx_head = next;
	} // si no hay espacio, se descarta el byte (simple y suficiente para este uso)
}
