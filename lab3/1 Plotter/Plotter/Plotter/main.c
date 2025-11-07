#define F_CPU 1000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

//								DEFINICIONES
////////////////////////////////////////////////////////////////////////////////////////////

#define motDelay	1      	// Delay en ms

//				Selector de máquina
/////////////////////////////////////////////////////
//	CNC Mateo
///////////////////
#define machine 0

#define clkX   (1<<PD2)
#define dirX   (1<<PD5)
#define enX    (1<<PB0)
#define sol    (1<<PB3)
#define clkY   (1<<PD3)
#define dirY   (1<<PD6)
#define enY    (1<<PB0)
#define limYA  (1<<PB2)
#define limYD  (1<<PB1)
#define LED    (1<<PB5)

/* Plotter lab config (kept for reference)
#define clkX   (1<<PB3)
#define dirX   (1<<PB4)
#define enX    (1<<PB5)
#define sol    (1<<PC0)
#define clkY   (1<<PC3)
#define dirY   (1<<PC4)
#define enY    (1<<PC5)
#define limYA  (1<<PD2)
#define limYD  (1<<PD3)
#define LED    (1<<PD5)
*/

//										FUNCIONES
////////////////////////////////////////////////////////////////////////////////////////////

// small helper
static inline void step_delay(void){
	_delay_ms(motDelay);
}

//					Movimientos
/////////////////////////////////////////////////////

//		Eje Y
///////////////////
void UP (int dist){
	if (machine == 0){		// CNC
		PORTD |= dirY;
		PORTB |= enY;
		for(int i = 0; i < dist; i++){
			PORTD |= clkY;
			step_delay();
			PORTD &= ~clkY;
			step_delay();
		}
	} else {		// Plotter branch (kept consistent)
		PORTB |= dirY;
		PORTB |= enY;
		for(int i = 0; i < dist; i++){
			PORTC |= clkY;
			step_delay();
			PORTC &= ~clkY;
			step_delay();
		}
	}
}

void DOWN (int dist){
	if (machine == 0){		// CNC
		PORTD &= ~dirY;
		PORTB |= enY;
		for(int i = 0; i < dist; i++){
			PORTD |= clkY;
			step_delay();
			PORTD &= ~clkY;
			step_delay();
		}
	} else {	// Plotter
		PORTB &= ~dirY;
		PORTB |= enY;
		for(int i = 0; i < dist; i++){
			PORTC |= clkY;
			step_delay();
			PORTC &= ~clkY;
			step_delay();
		}
	}
}

//		Eje X
/////////////////////

void L (int dist){
	if (machine == 0){		// CNC
		PORTD &= ~dirX;      // direction = LEFT (0)
		PORTB |= enX;
		for(int i = 0; i < dist; i++){
			PORTD |= clkX;
			step_delay();
			PORTD &= ~clkX;
			step_delay();
		}
	} else {	// Plotter
		PORTB &= ~dirX;
		PORTB |= enX;
		for(int i = 0; i < dist; i++){
			PORTB |= clkX;
			step_delay();
			PORTB &= ~clkX;
			step_delay();
		}
	}
}

void R (int dist){
	if (machine == 0){		// CNC
		PORTD |= dirX;       // direction = RIGHT (1)
		PORTB |= enX;
		for(int i = 0; i < dist; i++){
			PORTD |= clkX;
			step_delay();
			PORTD &= ~clkX;
			step_delay();
		}
	} else {	// Plotter
		PORTB |= dirX;
		PORTB |= enX;
		for(int i = 0; i < dist; i++){
			PORTB |= clkX;
			step_delay();
			PORTB &= ~clkX;
			step_delay();
		}
	}
}

//		Misc
/////////////////////

void solD(void){
	if (machine == 0){		// CNC
		PORTB |= sol;
		_delay_ms(50);
	} else {	
		PORTC |= sol;
		_delay_ms(50);
	}
}

void solU(void){
	if (machine == 0){		// CNC
		PORTB &= ~sol;
		_delay_ms(50);
	} else {	
		PORTC &= ~sol;
		_delay_ms(50);
	}
}

void ledON(void){
	if (machine == 0){		// CNC
		PORTB |= LED;
	} else {	// Plotter lab
		PORTD |= LED;
	}
}

void ledOFF(void){
	if (machine == 0){		// CNC
		PORTB &= ~LED;
	} else {	// Plotter lab
		PORTD &= ~LED;
	}
}

//					Autohome
//////////////////////////////////////////////////////
void home(void){
	if (machine == 0){		// CNC
		uint16_t d = 0;

		// flash LED
		PORTB |= LED; _delay_ms(300);
		PORTB &= ~LED; _delay_ms(150);
		PORTB |= LED; _delay_ms(300);
		PORTB &= ~LED; _delay_ms(200);

		solU();

		// Step down until lower-limit triggered (assumes active-high)
		while(!(PINB & limYA)){
			UP(1);
		}

		_delay_ms(500);

		// Step up and count until limit again (measure travel)
		while(!(PINB & limYD)){
			DOWN(1);
			d++;
		}

		_delay_ms(500);

		if (d > 0) UP(d/2);	// go to middle

		// flash LED end
		PORTB |= LED; _delay_ms(300);
		PORTB &= ~LED; _delay_ms(150);
		PORTB |= LED; _delay_ms(300);
		PORTB &= ~LED; _delay_ms(200);
	} else {	// Plotter lab
		uint16_t d = 0;

		PORTD |= LED; _delay_ms(300);
		PORTD &= ~LED; _delay_ms(150);
		PORTD |= LED; _delay_ms(300);
		PORTD &= ~LED; _delay_ms(200);

		solU();

		while(!(PIND & limYA) && !(PIND & limYD)){
			DOWN(1);
		}

		_delay_ms(500);

		while(!(PIND & limYA) && !(PIND & limYD)){
			UP(1);
			d++;
		}

		_delay_ms(500);

		if (d > 0) DOWN(d/2);

		PORTD |= LED; _delay_ms(300);
		PORTD &= ~LED; _delay_ms(150);
		PORTD |= LED; _delay_ms(300);
		PORTD &= ~LED; _delay_ms(200);
	}
}

/*
void circle(uint8_t radius){

}
*/

void cuadrado(uint16_t tam){
	solD();
	L(tam/2);
	UP(tam/2);
	R(tam);
	DOWN(tam);
	L(tam);
	UP(tam/2);
	solU();
	R(tam/2);
}

void cruz(uint16_t tam){
	solD();
	UP(tam/3);
	DOWN(tam/3);
	R(tam/3);
	L(tam/3);
	L(tam/3);
	R(tam/3);
	DOWN(tam*2/3);
	UP(tam*2/3);
	solU();
}

//										MAIN
////////////////////////////////////////////////////////////////////////////////////////////

int main (void){
	if (machine == 0){				// CNC
	    // Configure outputs: PD2 clkX, PD3 clkY, PD5 dirX, PD6 dirY
	    DDRD |= clkX | clkY | dirX | dirY;
	    // PortB: PB0 en, PB3 solenoid, PB5 LED output; PB1/PB2 inputs for limits
	    DDRB |= enX | sol | LED;
	    DDRB &= ~(limYA | limYD); 	// limit switches as INPUT
	    PORTB |= limYA | limYD; 	// enable pull-ups on switches
	} else {	// Plotter lab
	    DDRD |= clkX | clkY | dirX | dirY;
	    DDRB |= enX | sol | LED;
	    DDRB &= ~(limYA | limYD);
	    PORTB |= limYA | limYD;
	}

	// initial states
	PORTD &= ~(clkX | clkY | dirX | dirY);
	PORTB &= ~sol;
	PORTB &= ~enX;
	PORTB &= ~LED;

	// blink startup
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
  	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
  	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
  	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
  	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);

	// quick motion test (safe)
	//UP(2000); _delay_ms(2000); 	UP(200); _delay_ms(200); 
	//DOWN(200); _delay_ms(200); 	DOWN(200); _delay_ms(200);
	//L(200);
	//R(200);

	home();

	while (1){
		_delay_ms(1000);
	}

	return 0;
}
