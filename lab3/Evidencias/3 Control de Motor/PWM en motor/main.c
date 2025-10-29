#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void) {
	DDRB |= (1 << PB1);
	TCCR1A = (1 << WGM10) | (1 << COM1A1);
	TCCR1B = (1 << WGM12) | (1 << CS10);

	/* while (1) {
		for (uint8_t i = 77; i <= 102; i++) {
			uint8_t duty = (uint8_t)((255 * i) / 10);
			OCR1A = duty;
			_delay_ms(500);
		} 
	} */
	OCR1A = (uint8_t) 120;
}
