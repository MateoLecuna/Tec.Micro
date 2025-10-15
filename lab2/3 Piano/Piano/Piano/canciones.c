#include "canciones.h"
#include "figuras.h"
#include "hw_pins.h"
#include "sonidos.h"
#include "funciones.h"

void c_asesina(void){
	
	// Canción 1 - Asecina - Los Palmeras
	
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

void c_SuperMario(void){
	
	// Canción 2 - Super Mario Bros
	
	// 1
	play_note_ms(4, 1, SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(7, 1, SEMICORCHEA);
	delay_ms_var(CORCHEA + SEMICORCHEA);
	play_note_ms(7, 0, CORCHEA);
	delay_ms_var(CORCHEA);
	
	for (uint8_t i = 0; i < 2; i++){
		
		// 2
		play_note_ms(0, 1, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(7, 0, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(4, 0, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(11, 0, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(10, 0, SEMICORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		
		// 3
		play_note_ms(7, 0, SEMICORCHEA);
		play_note_ms(4, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
		play_note_ms(9, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(4, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(0, 1, SEMICORCHEA);
		play_note_ms(2, 1, SEMICORCHEA);
		play_note_ms(11, 0, SEMICORCHEA);
		delay_ms_var(CORCHEA);
	}
	
	for (uint8_t i = 0; i < 2; i++){
		// 4
		delay_ms_var(CORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
		play_note_ms(6, 1, SEMICORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(3, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(4, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(8, 0, SEMICORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(0, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(0, 1, SEMICORCHEA);
		play_note_ms(2, 1, SEMICORCHEA);
		
		// 5
		delay_ms_var(CORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
		play_note_ms(6, 1, SEMICORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(3, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(4, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(0, 2, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(0, 2, SEMICORCHEA);
		play_note_ms(0, 2, CORCHEA);
		delay_ms_var(CORCHEA);
		
		// 6
		delay_ms_var(CORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
		play_note_ms(6, 1, SEMICORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(3, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(4, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(8, 0, SEMICORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(0, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(0, 1, SEMICORCHEA);
		play_note_ms(2, 1, SEMICORCHEA);
		
		// 7
		delay_ms_var(CORCHEA);
		play_note_ms(3, 1, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(2, 1, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(0, 1, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(7, 0, SEMICORCHEA);
		play_note_ms(7, 0, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(7, 0, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
	}
	
	// 8
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(2, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(9, 0, SEMICORCHEA);
	play_note_ms(7, 0, CORCHEA);
	delay_ms_var(CORCHEA);
	
	// 9
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(2, 1, SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	delay_ms_var(BLANCA);
	
	// 8
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(2, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(9, 0, SEMICORCHEA);
	play_note_ms(7, 0, CORCHEA);
	delay_ms_var(CORCHEA);
	
	// 1
	play_note_ms(4, 1, SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(7, 1, CORCHEA);
	delay_ms_var(CORCHEA);
	play_note_ms(7, 0, CORCHEA);
	delay_ms_var(CORCHEA);
	
	for (uint8_t i = 0; i < 2; i++){
		
		// 2
		play_note_ms(0, 1, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(7, 0, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(4, 0, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(11, 0, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(10, 0, SEMICORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		
		// 3
		play_note_ms(7, 0, SEMICORCHEA);
		play_note_ms(4, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
		play_note_ms(9, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(7, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(4, 1, SEMICORCHEA);
		delay_ms_var(SEMICORCHEA);
		play_note_ms(0, 1, SEMICORCHEA);
		play_note_ms(2, 1, SEMICORCHEA);
		play_note_ms(11, 0, SEMICORCHEA);
		delay_ms_var(CORCHEA);
	}
	
	for (uint8_t i = 0; i < 2; i++){
		//10
		play_note_ms(4, 1, SEMICORCHEA);
		play_note_ms(0, 1, CORCHEA);
		play_note_ms(7, 0, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(8, 0, CORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(5, 1, CORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(9, 0, CORCHEA);
		delay_ms_var(CORCHEA);
		
		// 11
		play_note_ms(11, 0, SEMICORCHEA);
		play_note_ms(9, 1, CORCHEA);
		play_note_ms(9, 1, SEMICORCHEA);
		play_note_ms(9, 1, SEMICORCHEA);
		play_note_ms(7, 1, CORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(4, 1, SEMICORCHEA);
		play_note_ms(0, 1, CORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(7, 0, CORCHEA);
		delay_ms_var(CORCHEA);
		
		// 10
		play_note_ms(4, 1, SEMICORCHEA);
		play_note_ms(0, 1, CORCHEA);
		play_note_ms(7, 0, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(8, 0, CORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(5, 1, CORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(9, 0, CORCHEA);
		delay_ms_var(CORCHEA);
		
		// 12
		play_note_ms(11, 0, SEMICORCHEA);
		play_note_ms(5, 1, CORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(4, 1, CORCHEA);
		play_note_ms(2, 1, SEMICORCHEA);
		play_note_ms(0, 1, SEMICORCHEA);
		play_note_ms(2, 0, CORCHEA);
		play_note_ms(2, 0, SEMICORCHEA);
		play_note_ms(0, 1, CORCHEA);
		delay_ms_var(CORCHEA);
	}
	
	// 13
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(2, 1, CORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(9, 0, SEMICORCHEA);
	play_note_ms(7, 0, NEGRA);
	
	// 14
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(2, 1, SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	delay_ms_var(CORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	play_note_ms(7, 1, SEMICORCHEA);
	play_note_ms(4, 2, SEMICORCHEA);
	play_note_ms(0, 2, SEMICORCHEA);
	play_note_ms(2, 2, SEMICORCHEA);
	play_note_ms(7, 2, SEMICORCHEA);
	
	// 13
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(2, 1, CORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(9, 0, SEMICORCHEA);
	play_note_ms(7, 0, NEGRA);
	
	// 1
	play_note_ms(4, 1, SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(0, 1, SEMICORCHEA);
	play_note_ms(4, 1, SEMICORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(7, 1, SEMICORCHEA);
	delay_ms_var(CORCHEA + SEMICORCHEA);
	play_note_ms(7, 0, CORCHEA);
	delay_ms_var(CORCHEA);
	
	for (uint8_t i = 0; i < 2; i++){
		//10
		play_note_ms(4, 1, SEMICORCHEA);
		play_note_ms(0, 1, CORCHEA);
		play_note_ms(7, 0, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(8, 0, CORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(5, 1, CORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(9, 0, CORCHEA);
		delay_ms_var(CORCHEA);
		
		// 11
		play_note_ms(11, 0, SEMICORCHEA);
		play_note_ms(9, 1, CORCHEA);
		play_note_ms(9, 1, SEMICORCHEA);
		play_note_ms(9, 1, SEMICORCHEA);
		play_note_ms(7, 1, CORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(4, 1, SEMICORCHEA);
		play_note_ms(0, 1, CORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(7, 0, CORCHEA);
		delay_ms_var(CORCHEA);
		
		// 10
		play_note_ms(4, 1, SEMICORCHEA);
		play_note_ms(0, 1, CORCHEA);
		play_note_ms(7, 0, SEMICORCHEA);
		delay_ms_var(CORCHEA);
		play_note_ms(8, 0, CORCHEA);
		play_note_ms(9, 0, SEMICORCHEA);
		play_note_ms(5, 1, CORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(9, 0, CORCHEA);
		delay_ms_var(CORCHEA);
		
		// 12
		play_note_ms(11, 0, SEMICORCHEA);
		play_note_ms(5, 1, CORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(5, 1, SEMICORCHEA);
		play_note_ms(4, 1, CORCHEA);
		play_note_ms(2, 1, SEMICORCHEA);
		play_note_ms(0, 1, SEMICORCHEA);
		play_note_ms(2, 0, CORCHEA);
		play_note_ms(2, 0, SEMICORCHEA);
		play_note_ms(0, 1, CORCHEA);
		delay_ms_var(CORCHEA);
	}
	
	// Fin
	play_note_ms(0, 1, CORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(7, 0, CORCHEA);
	delay_ms_var(SEMICORCHEA);
	play_note_ms(4, 0, CORCHEA);
	play_note_ms(9, 0, SEMICORCHEA);
	play_note_ms(11, 0, CORCHEA);
	play_note_ms(9, 0, SEMICORCHEA);
	play_note_ms(8, 0, SEMICORCHEA);
	play_note_ms(10, 0, CORCHEA);
	play_note_ms(8, 0, SEMICORCHEA);
	play_note_ms(4, 0, SEMISEMICORCHEA);
	play_note_ms(2, 0, SEMISEMICORCHEA);
	play_note_ms(4, 0, NEGRA + CORCHEA);
	
		
}

void prueba (void){
	
}