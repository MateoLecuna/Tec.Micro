#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED1 PB5

void setup(void) {
	DDRB  |= (1 << DDB5);
	PORTB |= (1 << LED1);

	// Configurar el Timer 1 para generar
	// interrupciones cada 1 segundo
	TCCR1B |= (1 << WGM12);     // CTC
	OCR1A   = 15624;            // 1 s con prescaler 1024
	TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024
	TIMSK1 |= (1 << OCIE1A);    // Habilitar la interrupción
	sei();
}

void enter_sleep_mode(void) {
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_enable();
	sleep_cpu();       // Entrar en modo de reposo
	// La ejecución continúa aquí al despertar
	sleep_disable();   // Deshabilitar el modo de reposo
}

ISR(TIMER1_COMPA_vect) {
	PORTB ^= (1 << LED1);
}

int main(void) {
	setup();
	while (1) {
		enter_sleep_mode();
		// El microcontrolador estará en modo de reposo
		// hasta que se genere una interrupción
		// continuar código aquí (si hace falta)
	}
}
