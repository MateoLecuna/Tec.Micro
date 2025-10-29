#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

/* ==== UART ==== */
#define BAUD 115200
static void uart_init_baud(void){
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
	ADMUX=(1<<REFS0);
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);
}
static uint16_t adc_read(uint8_t ch){
	ADMUX=(ADMUX&0xF0)|(ch&0x0F);
	ADCSRA|=(1<<ADSC);
	while(ADCSRA&(1<<ADSC));
	return ADC;
}
static uint16_t adc_read_avg(uint8_t ch, uint8_t n){
	uint32_t acc=0; for(uint8_t i=0;i<n;i++) acc+=adc_read(ch);
	return (uint16_t)(acc/n);
}

/* ==== PWM (Timer1 OC1A = D9) ==== */
#define PIN_PWM_OC1A PB1
static void pwm_init_oc1a_fast8_presc1(void){
	DDRB |= (1 << PIN_PWM_OC1A);
	TCCR1A = (1 << WGM10) | (1 << COM1A1); // FastPWM 8-bit, no invertido
	TCCR1B = (1 << WGM12) | (1 << CS10);   // prescaler=1 (~62.5kHz)
	OCR1A = 0;
}

/* ==== Config ==== */
#define MIN_PWM   120
#define ADC_AVG_N 4
#define TS_MS     50

int main(void){
	uart_init_baud();
	adc_init();
	pwm_init_oc1a_fast8_presc1();

	uart_print("P3C,PWM fijo MIN_PWM\r\n");
	OCR1A = MIN_PWM;

	while(1){
		// solo reporte para verificar que el loop corre
		uart_print("pwm,"); uart_print_int(MIN_PWM); uart_print("\r\n");
		_delay_ms(TS_MS);
	}
}
