#include "timer.h"
#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>

void timer1_init_ctc_1khz(void){
	// Prescaler 64, OCR1A para 1kHz
	// tick = (64 / 16MHz) = 4 us
	// 1ms / 4us = 250 -> OCR1A = 249
	TCCR1A = 0;
	TCCR1B = (1<<WGM12);           // CTC
	OCR1A  = (uint16_t)((F_CPU / 64 / TICK_HZ) - 1); // 249 para 1kHz
	TIMSK1 = (1<<OCIE1A);
	TCCR1B |= (1<<CS11) | (1<<CS10); // prescaler 64
}

ISR(TIMER1_COMPA_vect){
	tick_1ms_isr_hook();
}
