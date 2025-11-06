#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdbool.h>

// Pines
#define PIN_PWM_OC1A   PB1   // D9 (OC1A)
#define LED_DIR_DER    PD4   // "DER"
#define LED_DIR_IZQ    PD5   // "IZQ"

// Configurables
#define TS_MS          1       // periodo de control (ms)
#define DEAD_ADC       10      // zona muerta en cuentas ADC
#define MIN_PWM        180     // umbral mínimo efectivo del PWM (MIN_PWM - 255)
#define KP_NUM         1       // ganancia proporcional
#define KP_DEN         4
#define ADC_AVG_N      4       // promedio (n muestras)

// UART (115200-8N1)
static void uart_init(uint16_t ubrr) {
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr & 0xFF);
	UCSR0B = (1 << TXEN0);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}
static void uart_putc(char c) {
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}
static void uart_print(const char *s) { while (*s) uart_putc(*s++); }
static void uart_print_int(int v) {
	char buf[12]; itoa(v, buf, 10); uart_print(buf);
}

// ADC
static void adc_init(void) {
	ADMUX  = (1 << REFS0);                 // AVcc como referencia
	ADCSRA = (1 << ADEN)                  // habilitar ADC
	| (1 << ADPS2) | (1 << ADPS1); // prescaler 64 - 250 kHz
}
static uint16_t adc_read(uint8_t ch) {
	ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC; // 10 bits
}
static uint16_t adc_read_avg(uint8_t ch, uint8_t n) {
	uint32_t acc = 0;
	for (uint8_t i=0; i<n; i++) acc += adc_read(ch);
	return (uint16_t)(acc / n);
}

// PWM Timer1 en D9 (OC1A)
static void pwm_init_oc1a_fast8_presc1(void) {
	DDRB  |= (1 << PIN_PWM_OC1A);                // D9 salida
	// Fast PWM 8-bit: WGM10=1, WGM12=1; salida no invertida en OC1A
	TCCR1A = (1 << WGM10) | (1 << COM1A1);
	TCCR1B = (1 << WGM12) | (1 << CS10);        // prescaler=1 - aprox. 62.5 kHz
	OCR1A = 0;
}

// LEDs dirección
static inline void dir_leds_stop(void){
	PORTD &= ~(1 << LED_DIR_DER);
	PORTD &= ~(1 << LED_DIR_IZQ);
}
static inline void dir_leds_DER(void){
	PORTD |=  (1 << LED_DIR_DER);
	PORTD &= ~(1 << LED_DIR_IZQ);
}
static inline void dir_leds_IZQ(void){
	PORTD &= ~(1 << LED_DIR_DER);
	PORTD |=  (1 << LED_DIR_IZQ);
}

// Main
int main(void) {
	// LEDs como salida
	DDRD |= (1 << LED_DIR_DER) | (1 << LED_DIR_IZQ);
	dir_leds_stop();

	adc_init();
	pwm_init_oc1a_fast8_presc1();

	// UART 115200 bps
	uart_init(8);
	uart_print("P3C,ref,act,pwm,dir\r\n");

	while (1) {
		// Lecturas filtradas
		uint16_t ref = adc_read_avg(0, ADC_AVG_N); // A0
		uint16_t act = adc_read_avg(1, ADC_AVG_N); // A1

		// Control proporcional + zona muerta
		int16_t e   = (int16_t)ref - (int16_t)act;
		uint16_t mag = (e >= 0) ? e : -e;

		uint8_t pwm = 0;
		const char *dir_str = "STOP";

		if (mag <= DEAD_ADC) {
			// dentro de la banda: freno virtual (sin puente H real)
			OCR1A = 0;
			dir_leds_stop();
			} else {
			// magnitud proporcional (saturada)
			uint16_t inc = (mag * KP_NUM) / KP_DEN;
			uint16_t raw = MIN_PWM + inc;
			if (raw > 255) raw = 255;
			pwm = (uint8_t)raw;

			if (e > 0) {         // ref > act “DER”
				dir_leds_DER();
				dir_str = "DER";
				} else {              // ref < act “IZQ”
				dir_leds_IZQ();
				dir_str = "IZQ";
			}
			OCR1A = pwm;          // velocidad
		}

		// Telemetría para gráfica
		
		uart_print_int(ref); uart_putc(',');
		uart_print_int(act); uart_putc(',');
		uart_print_int(pwm); uart_putc(',');
		uart_print(dir_str); uart_putc('\r'); uart_putc('\n');

		_delay_ms(TS_MS);
	}
}
