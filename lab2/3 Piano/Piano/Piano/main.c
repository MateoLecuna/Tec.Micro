#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "hw_pins.h"
#include "sonidos.h"
#include "figuras.h" 

static void delay_ms_var(uint16_t ms) {
	while (ms--) _delay_ms(1);
}

static void play_note_ms(uint8_t idx, int8_t octave, uint16_t ms) {
	sound_note_on(idx, octave);
	while (ms--) _delay_ms(1);
	sound_off();
	_delay_ms(10);
}

void c_asecina(){
	
	// Canción - Asecina - Los Palmeras
	
	// 1
	play_note_ms(4, 1, CORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	play_note_ms(9, 0, SEMICORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	delay_ms_var(CORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	play_note_ms(9, 0, SEMICORCHEA);
	
	// 2
	play_note_ms(11, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	delay_ms_var(CORCHEA);
	for (uint8_t i = 0; i < 2; i++){
		play_note_ms(11, 0, SEMICORCHEA);
		play_note_ms(0, 1, SEMICORCHEA);
		play_note_ms(11, 0, SEMICORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
	}
	play_note_ms(11, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(11, 0, CORCHEA);
	
	// 3
	play_note_ms(4, 1, CORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	for (uint8_t i = 0; i < 2; i++){
		play_note_ms(0, 1, SEMICORCHEA);
		play_note_ms(11, 0, SEMICORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(11, 0, SEMICORCHEA);
	}
	delay_ms_var(SEMICORCHEA);
	play_note_ms(11, 0, CORCHEA);
	
	// 4
	play_note_ms(3, 1, CORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	for (uint8_t i = 0; i < 2; i++){
		play_note_ms(0, 1, SEMICORCHEA);
		play_note_ms(11, 0, SEMICORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(11, 0, SEMICORCHEA);
	}
	delay_ms_var(SEMICORCHEA);
	play_note_ms(11, 0, CORCHEA);
	
	// 5
	play_note_ms(3, 1, CORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	play_note_ms(9, 0, SEMICORCHEA);
	play_note_ms(11, 0, CORCHEA);
	play_note_ms(9, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA),
	play_note_ms(7, 0, CORCHEA);
	play_note_ms(6, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	
	// 6
	play_note_ms(11, 1, CORCHEA);
	play_note_ms(7, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	for (uint8_t i = 0; i < 3; i++){
		play_note_ms(7, 1, SEMICORCHEA);
		play_note_ms(9, 1, SEMICORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
		play_note_ms(6, 1, SEMICORCHEA);
	}
	
	
	// 7
	play_note_ms(11, 1, CORCHEA);
	play_note_ms(7, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(7, 1, SEMICORCHEA);
	for (uint8_t i = 0; i < 2; i++){
		play_note_ms(9, 1, SEMICORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
		play_note_ms(6, 1, SEMICORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
	}
	delay_ms_var(SEMICORCHEA);
	play_note_ms(7, 1, CORCHEA);
	
	// 8
	play_note_ms(9, 1, CORCHEA);
	play_note_ms(6, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	for (uint8_t i = 0; i < 2; i++){
		play_note_ms(6, 1, SEMICORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
		play_note_ms(6, 1, SEMICORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
	}
	play_note_ms(6, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(6, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	
	// 9
	play_note_ms(9, 1, CORCHEA);
	play_note_ms(6, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	for (uint8_t i = 0; i < 3; i++){
		play_note_ms(6, 1, SEMICORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
		play_note_ms(6, 1, SEMICORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
	}
	
	// 10
	play_note_ms(0, 1, SEMICORCHEA + CORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(7, 0, SEMICORCHEA);
	play_note_ms(9, 0, SEMICORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	play_note_ms(2, 1, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA + CORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(7, 0, SEMISEMICORCHEA);
	play_note_ms(8, 0, SEMISEMICORCHEA);
	play_note_ms(9, 0, SEMISEMICORCHEA);
	play_note_ms(10, 0, SEMISEMICORCHEA);
	play_note_ms(11, 0, SEMISEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	
	// 11
	play_note_ms(11, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, CORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, CORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, CORCHEA);
	play_note_ms(11, 0, SEMICORCHEA);
	delay_ms_var(CORCHEA);
	
	// 12
	play_note_ms(11, 0, SEMISEMICORCHEA);
	play_note_ms(0, 1, SEMISEMICORCHEA);
	play_note_ms(2, 1, SEMISEMICORCHEA);
	play_note_ms(4, 1, SEMISEMICORCHEA + REDONDA);
	delay_ms_var(REDONDA);
}

int main(void) {
	hw_init_pins();
	sound_init_timer2();

	while (1) {
		c_asecina();
	}
}
