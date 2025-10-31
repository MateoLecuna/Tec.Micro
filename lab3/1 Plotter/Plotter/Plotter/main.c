#define F_CPU 16000000L

#include <avr/io.h>
#include <util/delay.h>



//								DEFINICIONES
////////////////////////////////////////////////////////////////////////////////////////////

#define PASO1DWN	0x28
#define PASO2DWN	0x28
#define PASO1UP		0x38
#define PASO2UP		0x30

#define PASO1L		0x28
#define PASO2L		0x28
#define PASO1R		0x38
#define PASO2R		0x30


#define motDelay	10


////////////////////////////////////////////////////////////////////////////////////////////


void home(void);






//										MAIN
////////////////////////////////////////////////////////////////////////////////////////////

int main (void){
	DDRD = 0x20;
	PORTD = 0;
	
	while(1){
		if (!(PIND & 0x04)){
			PORTD = 0x20;
			} else {
			PORTD = 0;
		}
	}
}








//										FUNCIONES
////////////////////////////////////////////////////////////////////////////////////////////


void UP (int dist){
	for(int i = 0 ; i <= dist ; i++){
		PORTC |= PASO1UP;
		_delay_ms(motDelay);
		PORTC &= 0xF7				// ~0b0000 1000 (CLK Y)
		_delay_ms(motDelay);
	}
}

void DWN (int dist){
	for(int i = 0 ; i <= dist ; i++){
		PORTC |= PASO1DWN;
		_delay_ms(motDelay);
		PORTC &= 0xF7				// ~0b0000 1000 (CLK Y)
		_delay_ms(motDelay);
	}
}


void home(void){
	while((PIND & 0x04)){
		if (!(PIND & 0x04)){
			PORTD = 0x20;
			} else {
			PORTD = 0;
		}
	}
}
