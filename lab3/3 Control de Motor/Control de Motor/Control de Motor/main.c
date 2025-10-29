#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

/* ==== UART ==== */
#define BAUD 115200
static void uart_init_baud(void) {
	UCSR0A = (1 << U2X0);
	uint16_t ubrr = (uint16_t)(F_CPU / (8UL * BAUD) - 1UL);
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr & 0xFF);
	UCSR0B = (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
static void uart_putc(char c){ while(!(UCSR0A & (1<<UDRE0))); UDR0 = c; }
static void uart_print(const char *s){ while(*s) uart_putc(*s++); }
static void uart_print_int(int v){ char b[12]; itoa(v,b,10); uart_print(b); }

/* ==== ADC ==== */
static void adc_init(void){
	ADMUX  = (1 << REFS0);                 // AVcc
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // presc=64
}
static uint16_t adc_read(uint8_t ch){
	ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC; // 10 bits
}
static uint16_t adc_read_avg(uint8_t ch, uint8_t n){
	uint32_t acc=0; for(uint8_t i=0;i<n;i++) acc+=adc_read(ch);
	return (uint16_t)(acc/n);
}

/* ==== Config ==== */
#define TS_MS     20
#define ADC_AVG_N 4

int main(void){
	uart_init_baud();
	adc_init();
	uart_print("P3C,ref(A0),act(A1)\r\n");
	while(1){
		uint16_t ref = adc_read_avg(0, ADC_AVG_N);
		uint16_t act = adc_read_avg(1, ADC_AVG_N);
		uart_print_int(ref); uart_putc(',');
		uart_print_int(act); uart_putc('\r'); uart_putc('\n');
		_delay_ms(TS_MS);
	}
}
