#define F_CPU 1000000L

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>



//								DEFINICIONES
////////////////////////////////////////////////////////////////////////////////////////////

#define PASO1DWN	0x28
#define PASO2DWN	0x20
#define PASO1UP		0x38
#define PASO2UP		0x30

#define PASO1L		0x28
#define PASO2L		0x20
#define PASO1R		0x38
#define PASO2R		0x30

#define LED 		0x20

#define motDelay	1      	// Delay en ms


//				Selector de máquina
/////////////////////////////////////////////////////
//	CNC Mateo
///////////////////
#define machine 0

#define clkX 	PD2
#define dirX 	PD5
#define enX 	PB0
#define sol 	PB3
#define clkY	PD3
#define dirY	PD6
#define enY		PB0
#define limYA	PB2
#define limYD	PB1
#define LED		PB5


//	Plotter lab
///////////////////
/*
#define machine 1

#define clkX 	PB3
#define dirX 	PB4
#define enX		PB5
#define sol		PC0
#define clkY	PC3
#define dirY	PC4
#define enY		PC5
#define limYA	PD2
#define limYD	PD3
#define LED		PD5
*/

//										FUNCIONES
////////////////////////////////////////////////////////////////////////////////////////////

//					Movimientos
/////////////////////////////////////////////////////

//		Eje Y
///////////////////
void UP (int dist){
	if (machine == 0){		// Chequea qué máquina es
		PORTD =| dirY;
		PORTB =| enY;
		for(int i = 0 ; i <= dist ; i++){
			PORTD =| clkY;
			_delay_ms(1);
			PORTD =& ~clkY;
			_delay_ms(1);
		}
	} else if (machine == 1){		
		PORTB =| dirY;
		PORTB =| enY;
		for(int i = 0 ; i <= dist ; i++){
			PORTC =| clkY;
			_delay_ms(1);
			PORTC =& ~clkY;
			_delay_ms(1);
		}
	}

}



void DOWN (int dist){
	if (machine == 0){		// Chequea qué máquina es
		PORTD =& ~dirY;
		PORTB =| enY;
		for(int i = 0 ; i <= dist ; i++){
			PORTD =| clkY;
			_delay_ms(1);
			PORTD =& ~clkY;
			_delay_ms(1);
		}
	} else if (machine == 1){	
		PORTD =& ~dirY;
		PORTB =| enY;
		for(int i = 0 ; i <= dist ; i++){
			PORTC =| clkY;
			_delay_ms(1);
			PORTC =& ~clkY;
			_delay_ms(1);
		}
	}

}

//		Eje X
/////////////////////

void L (int dist){
	if (machine == 0){		// Chequea qué máquina es
		PORTD =& ~dirX;
		PORTB =| enX;
		for(int i = 0 ; i <= dist ; i++){
			PORTD =| clkX;
			_delay_ms(1);
			PORTD =& ~clkX;
			_delay_ms(1);
		}
		
	} else if (machine == 1){	
		PORTB =& ~dirX;
		PORTB =| enX;
		for(int i = 0 ; i <= dist ; i++){
			PORTB =| clkX;
			_delay_ms(1);
			PORTB =& ~clkX;
			_delay_ms(1);
		}
	}
}

void R (int dist){
	if (machine == 0){		// Chequea qué máquina es
		PORTD =& ~dirX;
		PORTB =| enX;
		for(int i = 0 ; i <= dist ; i++){
			PORTD =| clkX;
			_delay_ms(1);
			PORTD =& ~clkX;
			_delay_ms(1);
		}
		
	} else if (machine == 1){	
		PORTB =| dirX;
		PORTB =| enX;
		for(int i = 0 ; i <= dist ; i++){
			PORTB =| clkX;
			_delay_ms(1);
			PORTB =& ~clkX;
			_delay_ms(1);
		}
	}
}

//		Misc
/////////////////////

void solD(void){
	if (machine == 0){		// Chequea qué máquina es
		PORTB |= sol;
		_delay_ms(50);
	}
		
	} else if (machine == 1){	
		PORTC |= sol;
		_delay_ms(50);
	}
}



void solU(void){
	if (machine == 0){		// Chequea qué máquina es
		PORTB &= ~sol;
		_delay_ms(50);
	}
		
	} else if (machine == 1){	
		PORTC &= ~sol;
		_delay_ms(50);
	}
}

void ledON(void){
	if (machine == 0){		// CNC
		PORTB |= led;
	}
		
	} else if (machine == 1){	// Plotter lab
		PORTD |= led;
	}

}

void ledOFF(void){
	if (machine == 0){		// CNC
		PORTB &= ~led;
	}
		
	} else if (machine == 1){	// Plotter lab
		PORTD &= ~led;
	}
}


//					Autohome
//////////////////////////////////////////////////////
void home(void){

	if (machine == 0){		// CNC
		uint16_t d = 0;
	
		
		PORTB |= LED;			// Prende y apaga el LED
		_delay_ms(300);
		PORTB &= ~LED;
		_delay_ms(150);
		PORTB |= LED;
		_delay_ms(300);
		PORTB &= ~LED;
		_delay_ms(200);
	
		solU();
		
		while(!(PINB & limYA || PINB & limYB)){		// Paso 1: Baja hasta encontrar el limite inferior (YA)
				DOWN(1);
		}
	
		_delay_ms(500);
		
		while(!(PINB & limYA || PINB & limYB)){		// Paso 1: Baja hasta encontrar el limite inferior (YA)
				UP(1);
				d++;
		}
		
		_delay_ms(500);
		
		DOWN(d/2);	// Paso 3: Baja hasta la mitad del trayecto para ubicarse en el centro
		
	
		PORTB |= LED;			// Prende y apaga el LED
		_delay_ms(300);
		PORTB &= ~LED;
		_delay_ms(150);
		PORTB |= LED;
		_delay_ms(300);
		PORTB &= ~LED;
		_delay_ms(200);


		
	}








	
	} else if (machine == 1){	// Plotter lab
		uint16_t d = 0;
	
		
		PORTD |= LED;			// Prende y apaga el LED
		_delay_ms(300);
		PORTD &= ~LED;
		_delay_ms(150);
		PORTD |= LED;
		_delay_ms(300);
		PORTD &= ~LED;
		_delay_ms(200);
	
		solU();
		
		while(!(PIND & limYA || PIND & limYB)){		// Paso 1: Baja hasta encontrar el limite inferior (YA)
				DOWN(1);
		}
	
		_delay_ms(500);
		
		while(!(PIND & limYA || PIND & limYB)){		// Paso 1: Baja hasta encontrar el limite inferior (YA)
				UP(1);
				d++;
		}
		
		_delay_ms(500);
		
		DOWN(d/2);	// Paso 3: Baja hasta la mitad del trayecto para ubicarse en el centro
		
	
		PORTD |= LED;			// Prende y apaga el LED
		_delay_ms(300);
		PORTD &= ~LED;
		_delay_ms(150);
		PORTD |= LED;
		_delay_ms(300);
		PORTD &= ~LED;
		_delay_ms(200);

	}



	
}





//			Probador de frecuencias    (CÓDIGO VIEJO)
//////////////////////////////////////////////////////

void pruebaFrecuencias(void){
	for (uint16_t time = 2 ; time <= 10 ; time++){		// De 500 Hz a 100 Hz
		for (uint8_t rep = 0; rep < 20 ; rep++){			// Da 20 pasos con la frecuencia estimada
			PORTC |= PASO1UP;			// Pone en 1 todos los bits que mueven el motor
			for (uint8_t n = 0 ; n < time ; n++){
				_delay_ms(1);
			}
			PORTC &= 0xF7;				// ~0b0000 1000 (CLK Y) Apaga la señal de reloj
			for (uint8_t n = 0 ; n < time ; n++){
				_delay_ms(1);
			}
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

	for (uint8_t x = 0 ; x < radius ; x++){			// Cuadrante 1
		R(1);
		for (uint8_t j = 0; j <= sqrt((pow(radius,2))-(pow((x-radius),2))) ; j++){
			UP(1);
		}
	}

	for (uint8_t x = 0 ; x < radius ; x++){			// Cuadrante 2
		R(1);
		DOWN(sqrt((pow(radius,2))-(pow((x-radius),2))));
	}

	for (uint8_t x = 0 ; x < radius ; x++){			// Cuadrante 3
		L(1);
		DOWN(sqrt((pow(radius,2))-(pow((x-radius),2))));
	}

	for (uint8_t x = 0 ; x < radius ; x++){			// Cuadrante 4
		L(1);
		UP(sqrt((pow(radius,2))-(pow((x-radius),2))));
	}

	
}


void cuadrado(uint16_t tam){
	solD;
	L(tam/2);
	UP(tam/2);
	R(tam);
	DOWN(tam);
	L(tam);
	UP(tam/2);
	solUP;
	R(tam/2)
}


void cruz(uint16_t tam){
	solD;
	UP(tam/3);
	DOWN(tam/3);
	R(tam/3);
	L(tam/3);
	L(tam/3);
	R(tam/3);
	DOWN(tam*2/3);
	UP(tam*2/3);
	solU;
}










//										MAIN
////////////////////////////////////////////////////////////////////////////////////////////

int main (void){
	
	ledON();
	_delay_ms(50);
	ledOFF();
	_delay_ms(50);
	ledON();
	_delay_ms(50);
	ledOFF();
	_delay_ms(50);
	ledON();
	_delay_ms(50);
	ledOFF();
	_delay_ms(50);
	ledON();
	_delay_ms(50);
	ledOFF();
	_delay_ms(50);





	
	UP(200);
	_delay_ms(200);
	DOWN(200);
	L(200);
	R(200);



	
	
	home();

	

	
}
