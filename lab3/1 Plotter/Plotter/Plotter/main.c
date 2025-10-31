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

#define LED 		0x20


#define motDelay	10





//										FUNCIONES
////////////////////////////////////////////////////////////////////////////////////////////

//					Movimientos
/////////////////////////////////////////////////////

//		Eje Y
///////////////////
void UP (int dist){
	for(int i = 0 ; i <= dist ; i++){
		if (PIND & 0x08){				// Chequea que el carro Y no supere el limite YD
			PORTC |= PASO1UP;			// Pone en 1 todos los bits que mueven el motor
			_delay_ms(motDelay);
			PORTC &= 0xF7				// ~0b0000 1000 (CLK Y) Apaga la señal de reloj
			_delay_ms(motDelay);
		} else {
			break;
		}
	}
}

void DWN (int dist){
	for(int i = 0 ; i <= dist ; i++){	// Da la cantidad de pasos definida por el usuario
		if (PIND & 0x04){				// Chequea que el carro Y no supere el limite YA
			PORTC |= PASO1DWN;
			_delay_ms(motDelay);
			PORTC &= 0xF7				// ~0b0000 1000 (CLK Y)
			_delay_ms(motDelay);
		} else {
			break;
		}
	}
}

//		Eje X
/////////////////////

void L (int dist){
	for(int i = 0 ; i <= dist ; i++){
			PORTB |= PASO1L;			// Pone en 1 todos los bits que mueven el motor
			_delay_ms(motDelay);
			PORTB &= 0xF7				// ~0b0000 1000 (CLK Y) Apaga la señal de reloj
			_delay_ms(motDelay);
	}
}

void R (int dist){
	for(int i = 0 ; i <= dist ; i++){
			PORTB |= PASO1R;			// Pone en 1 todos los bits que mueven el motor
			_delay_ms(motDelay);
			PORTB &= 0xF7				// ~0b0000 1000 (CLK Y) Apaga la señal de reloj
			_delay_ms(motDelay);
	}
}


//					Autohome
//////////////////////////////////////////////////////
void home(void){
	uint16_t d = 0;

	
	PORTD |= LED;			// Prende y apaga el LED
	_delay_ms(300);
	PORTD &= ~LED;
	_delay_ms(150);
	PORTD |= LED;	
	_delay_ms(300);
	PORTD &= ~LED;
	_delay_ms(1000);

	
	while((PIND & 0x04)){		// Paso 1: Baja hasta encontrar el limite inferior (YA)
		if ((PIND & 0x04)){
			DOWN(1);
		} else {
			break;
		}
	}

	_delay_ms(300);
	
	while((PIND & 0x08)){		// Paso 2: Sube hasta encontrar el limite superior (YD)
		if ((PIND & 0x08)){
			UP(1);
			d++					// Recopila la distancia recorrida
		} else {
			break;
		}
	}
	
	_delay_ms(300);
	
	for (int i = 0 ; i <= (d/2) ; i++){		// Paso 3: Baja hasta la mitad del trayecto para ubicarse en el centro
		DOWN(1);
	}
	

		PORTD |= LED;			// Prende y apaga el LED
	_delay_ms(300);
	PORTD &= ~LED;
	_delay_ms(150);
	PORTD |= LED;	
	_delay_ms(300);
	PORTD &= ~LED;
	_delay_ms(1000);

	
}
















//										MAIN
////////////////////////////////////////////////////////////////////////////////////////////

int main (void){
	DDRB = 0xFF;
	DDRC = 0xFF;
	DDRD = 0xF3;

	PORTB = 0;
	PORTC = 0;
	PORTD = 0;

	home();

	

	
}






