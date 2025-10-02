#include "uart.h"
#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define TX_BUF_SIZE 128
#define RX_BUF_SIZE 64
#define TX_MASK (TX_BUF_SIZE-1)
#define RX_MASK (RX_BUF_SIZE-1)

static volatile uint8_t tx_buf[TX_BUF_SIZE];
static volatile uint8_t rx_buf[RX_BUF_SIZE];
static volatile uint8_t tx_head=0, tx_tail=0;
static volatile uint8_t rx_head=0, rx_tail=0;

void uart_init(uint32_t baud){
	uint16_t ubrr = (F_CPU/16/baud) - 1;
	UBRR0H = (uint8_t)(ubrr>>8);
	UBRR0L = (uint8_t)(ubrr);

	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00); // 8N1
}

bool uart_tx_enqueue(uint8_t b){
	uint8_t next = (tx_head + 1) & TX_MASK;
	if (next == tx_tail) return false; // full
	tx_buf[tx_head] = b;
	tx_head = next;
	UCSR0B |= (1<<UDRIE0); // habilita ISR UDRE
	return true;
}

bool uart_rx_dequeue(uint8_t* b){
	if (rx_head == rx_tail) return false; // vacío
	*b = rx_buf[rx_tail];
	rx_tail = (rx_tail + 1) & RX_MASK;
	return true;
}

void uart_print(const char* s){
	while(*s){
		while(!uart_tx_enqueue((uint8_t)*s)) { /* esperar espacio */ }
		s++;
	}
}

ISR(USART_RX_vect){
	uint8_t d = UDR0;
	uint8_t next = (rx_head + 1) & RX_MASK;
	if (next != rx_tail) { rx_buf[rx_head] = d; rx_head = next; }
}

ISR(USART_UDRE_vect){
	if (tx_head == tx_tail){
		UCSR0B &= ~(1<<UDRIE0); // buffer vacío
		return;
	}
	UDR0 = tx_buf[tx_tail];
	tx_tail = (tx_tail + 1) & TX_MASK;
}
