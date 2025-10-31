#define F_CPU 16000000L

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>



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

void DOWN (int dist){
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

//		Misc
/////////////////////

void solD(void){
	PORTC |= 0x01:
	_delay_ms(50);
}

void solU(void){
	PORTC &= ~0x01:
	_delay_ms(50);
}

void ledON(void){
	PORTD |= 0x20:
}

void ledOFF(void){
	PORTD &= ~0x20:
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

	solU();
	
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
		DOWN(1);							// (Tambien se puede escribir DOWN(d/2);
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





//			Probador de frecuencias
//////////////////////////////////////////////////////

void pruebaFrecuencias(void){
	for (uint16_t time = 2 ; tiempo <= 10 ; tiempo++){		// De 500 Hz a 100 Hz
		for (uint8_t rep = 0; rep < 20 ; rep++){			// Da 20 pasos con la frecuencia estimada
			PORTC |= PASO1UP;			// Pone en 1 todos los bits que mueven el motor
			_delay_ms(time);
			PORTC &= 0xF7				// ~0b0000 1000 (CLK Y) Apaga la señal de reloj
			_delay_ms(time);
		}
	}

}



//					Figuras
//////////////////////////////////////////////////////
//		Circulo
/////////////////////
void circle (uint8_t radius){
	solU();
	L(radius);
	solD();

	for (uint8_t x ; x < radius ; x++){			// Cuadrante 1
		R(1);
		for (uint8_t j ; j <= sqrt((pow(radius,2))-(pow((x-radius),2))) ; j++){
			UP(1);
		}
	}

	for (uint8_t x ; x < radius ; x++){			// Cuadrante 2
		R(1);
		DOWN(sqrt((pow(radius,2))-(pow((x-radius),2))));
	}

	for (uint8_t x ; x < radius ; x++){			// Cuadrante 3
		L(1);
		DOWN(sqrt((pow(radius,2))-(pow((x-radius),2))));
	}

	for (uint8_t x ; x < radius ; x++){			// Cuadrante 4
		L(1);
		UP(sqrt((pow(radius,2))-(pow((x-radius),2))));
	}

	
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






