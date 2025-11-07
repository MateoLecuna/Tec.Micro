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
void U (int dist){
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

void D (int dist){
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
		PORTD &= ~dirX;      // direction = L (0)
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
		PORTD |= dirX;       // direction = R (1)
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



//////////////////////////////////////////////////////


//////////////////////////////////////////////////////

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

		// Step D until lower-limit triggered (assumes active-high)
		while(!(PINB & limYA)){
			U(1);
		}

		_delay_ms(500);

		// Step U and count until limit again (measure travel)
		while(!(PINB & limYD)){
			D(1);
			d++;
		}

		_delay_ms(500);

		if (d > 0) U(d/2);	// go to middle

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
			D(1);
		}

		_delay_ms(500);

		while(!(PIND & limYA) && !(PIND & limYD)){
			U(1);
			d++;
		}

		_delay_ms(500);

		if (d > 0) D(d/2);

		PORTD |= LED; _delay_ms(300);
		PORTD &= ~LED; _delay_ms(150);
		PORTD |= LED; _delay_ms(300);
		PORTD &= ~LED; _delay_ms(200);
	}
}

/* Old frequency tester removed (referenced undefined symbols).
   Commented out to keep the file clean. */

/* Circle function uses floating math and was requested to be ignored for now.
   Implement Bresenham-based circle later. */

/*
void circle(uint8_t radius){
    // implement Bresenham later
}
*/

void cuadrado(uint16_t tam){
	solD();
	L(tam/2);
	U(tam/2);
	R(tam);
	D(tam);
	L(tam);
	U(tam/2);
	solU();
	R(tam/2);
}

void cruz(uint16_t tam){
	solD();
	U(tam/3);
	D(tam/3);
	R(tam/3);
	L(tam/3);
	L(tam/3);
	R(tam/3);
	D(tam*2/3);
	U(tam*2/3);
	solU();
}


void triangulo(uint16_t tam){
  solD();
  for(int i = 0 ; i < (tam/2) ; i++){
    L(1);
    U(2);
  }
    for(int i = 0 ; i < (tam/2) ; i++){
    L(1);
    D(2);
  }
  R(tam);
}


void circulo(void) {
  L(1);D(87);
  L(2);D(87);
  L(4);D(88);
  L(5);D(87);
  L(7);D(87);
  L(8);D(87);
  L(10);D(86);
  L(12);D(87);
  L(13);D(86);
  L(14);D(86);
  L(16);D(86);
  L(17);D(86);
  L(19);D(85);
  L(21);D(85);
  L(21);D(84);
  L(24);D(84);
  L(24);D(84);
  L(27);D(83);
  L(27);D(83);
  L(30);D(82);
  L(30);D(82);
  L(32);D(81);
  L(33);D(81);
  L(35);D(80);
  L(36);D(79);
  L(38);D(79);
  L(39);D(78);
  L(40);D(77);
  L(42);D(77);
  L(43);D(76);
  L(44);D(75);
  L(46);D(75);
  L(47);D(73);
  L(48);D(73);
  L(49);D(72);
  L(51);D(71);
  L(52);D(70);
  L(53);D(69);
  L(54);D(69);
  L(56);D(67);
  L(56);D(66);
  L(58);D(66);
  L(59);D(64);
  L(60);D(63);
  L(61);D(63);
  L(63);D(61);
  L(63);D(60);
  L(64);D(59);
  L(66);D(58);
  L(66);D(56);
  L(67);D(56);
  L(69);D(54);
  L(69);D(53);
  L(70);D(52);
  L(71);D(51);
  L(72);D(49);
  L(73);D(48);
  L(73);D(47);
  L(75);D(46);
  L(75);D(44);
  L(76);D(43);
  L(77);D(42);
  L(77);D(40);
  L(78);D(39);
  L(79);D(38);
  L(79);D(36);
  L(80);D(35);
  L(81);D(33);
  L(81);D(32);
  L(82);D(30);
  L(82);D(30);
  L(83);D(27);
  L(83);D(27);
  L(84);D(24);
  L(84);D(24);
  L(84);D(21);
  L(85);D(21);
  L(85);D(19);
  L(86);D(17);
  L(86);D(16);
  L(86);D(14);
  L(86);D(13);
  L(87);D(12);
  L(86);D(10);
  L(87);D(8);
  L(87);D(7);
  L(87);D(5);
  L(88);D(4);
  L(87);D(2);
  L(87);D(1);
  L(86);U(1);
  L(87);U(2);
  L(88);U(4);
  L(87);U(5);
  L(87);U(7);
  L(87);U(8);
  L(86);U(10);
  L(87);U(12);
  L(86);U(13);
  L(86);U(14);
  L(86);U(16);
  L(86);U(17);
  L(85);U(19);
  L(85);U(21);
  L(84);U(21);
  L(84);U(24);
  L(84);U(24);
  L(83);U(27);
  L(83);U(27);
  L(82);U(30);
  L(82);U(30);
  L(81);U(32);
  L(81);U(33);
  L(80);U(35);
  L(79);U(36);
  L(79);U(38);
  L(78);U(39);
  L(77);U(40);
  L(77);U(42);
  L(76);U(43);
  L(75);U(44);
  L(75);U(46);
  L(73);U(47);
  L(73);U(48);
  L(72);U(49);
  L(71);U(51);
  L(70);U(52);
  L(69);U(53);
  L(69);U(54);
  L(67);U(56);
  L(66);U(56);
  L(66);U(58);
  L(64);U(59);
  L(63);U(60);
  L(63);U(61);
  L(61);U(63);
  L(60);U(63);
  L(59);U(64);
  L(58);U(66);
  L(56);U(66);
  L(56);U(67);
  L(54);U(69);
  L(53);U(69);
  L(52);U(70);
  L(51);U(71);
  L(49);U(72);
  L(48);U(73);
  L(47);U(73);
  L(46);U(75);
  L(44);U(75);
  L(43);U(76);
  L(42);U(77);
  L(40);U(77);
  L(39);U(78);
  L(38);U(79);
  L(36);U(79);
  L(35);U(80);
  L(33);U(81);
  L(32);U(81);
  L(30);U(82);
  L(30);U(82);
  L(27);U(83);
  L(27);U(83);
  L(24);U(84);
  L(24);U(84);
  L(21);U(84);
  L(21);U(85);
  L(19);U(85);
  L(17);U(86);
  L(16);U(86);
  L(14);U(86);
  L(13);U(86);
  L(12);U(87);
  L(10);U(86);
  L(8);U(87);
  L(7);U(87);
  L(5);U(87);
  L(4);U(88);
  L(2);U(87);
  L(1);U(87);
  R(1);U(86);
  R(2);U(87);
  R(4);U(88);
  R(5);U(87);
  R(7);U(87);
  R(8);U(87);
  R(10);U(86);
  R(12);U(87);
  R(13);U(86);
  R(14);U(86);
  R(16);U(86);
  R(17);U(86);
  R(19);U(85);
  R(21);U(85);
  R(21);U(84);
  R(24);U(84);
  R(24);U(84);
  R(27);U(83);
  R(27);U(83);
  R(30);U(82);
  R(30);U(82);
  R(32);U(81);
  R(33);U(81);
  R(35);U(80);
  R(36);U(79);
  R(38);U(79);
  R(39);U(78);
  R(40);U(77);
  R(42);U(77);
  R(43);U(76);
  R(44);U(75);
  R(46);U(75);
  R(47);U(73);
  R(48);U(73);
  R(49);U(72);
  R(51);U(71);
  R(52);U(70);
  R(53);U(69);
  R(54);U(69);
  R(56);U(67);
  R(56);U(66);
  R(58);U(66);
  R(59);U(64);
  R(60);U(63);
  R(61);U(63);
  R(63);U(61);
  R(63);U(60);
  R(64);U(59);
  R(66);U(58);
  R(66);U(56);
  R(67);U(56);
  R(69);U(54);
  R(69);U(53);
  R(70);U(52);
  R(71);U(51);
  R(72);U(49);
  R(73);U(48);
  R(73);U(47);
  R(75);U(46);
  R(75);U(44);
  R(76);U(43);
  R(77);U(42);
  R(77);U(40);
  R(78);U(39);
  R(79);U(38);
  R(79);U(36);
  R(80);U(35);
  R(81);U(33);
  R(81);U(32);
  R(82);U(30);
  R(82);U(30);
  R(83);U(27);
  R(83);U(27);
  R(84);U(24);
  R(84);U(24);
  R(84);U(21);
  R(85);U(21);
  R(85);U(19);
  R(86);U(17);
  R(86);U(16);
  R(86);U(14);
  R(86);U(13);
  R(87);U(12);
  R(86);U(10);
  R(87);U(8);
  R(87);U(7);
  R(87);U(5);
  R(88);U(4);
  R(87);U(2);
  R(86);U(1);
  R(87);D(1);
  R(87);D(2);
  R(88);D(4);
  R(87);D(5);
  R(87);D(7);
  R(87);D(8);
  R(86);D(10);
  R(87);D(12);
  R(86);D(13);
  R(86);D(14);
  R(86);D(16);
  R(86);D(17);
  R(85);D(19);
  R(85);D(21);
  R(84);D(21);
  R(84);D(24);
  R(84);D(24);
  R(83);D(27);
  R(83);D(27);
  R(82);D(30);
  R(82);D(30);
  R(81);D(32);
  R(81);D(33);
  R(80);D(35);
  R(79);D(36);
  R(79);D(38);
  R(78);D(39);
  R(77);D(40);
  R(77);D(42);
  R(76);D(43);
  R(75);D(44);
  R(75);D(46);
  R(73);D(47);
  R(73);D(48);
  R(72);D(49);
  R(71);D(51);
  R(70);D(52);
  R(69);D(53);
  R(69);D(54);
  R(67);D(56);
  R(66);D(56);
  R(66);D(58);
  R(64);D(59);
  R(63);D(60);
  R(63);D(61);
  R(61);D(63);
  R(60);D(63);
  R(59);D(64);
  R(58);D(66);
  R(56);D(66);
  R(56);D(67);
  R(54);D(69);
  R(53);D(69);
  R(52);D(70);
  R(51);D(71);
  R(49);D(72);
  R(48);D(73);
  R(47);D(73);
  R(46);D(75);
  R(44);D(75);
  R(43);D(76);
  R(42);D(77);
  R(40);D(77);
  R(39);D(78);
  R(38);D(79);
  R(36);D(79);
  R(35);D(80);
  R(33);D(81);
  R(32);D(81);
  R(30);D(82);
  R(30);D(82);
  R(27);D(83);
  R(27);D(83);
  R(24);D(84);
  R(24);D(84);
  R(21);D(84);
  R(21);D(85);
  R(19);D(85);
  R(17);D(86);
  R(16);D(86);
  R(14);D(86);
  R(13);D(86);
  R(12);D(87);
  R(10);D(86);
  R(8);D(87);
  R(7);D(87);
  R(5);D(87);
  R(4);D(88);
  R(2);D(87);
  R(1);D(86);
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
	    PORTB |= limYA | limYD; 	// enable pull-Us on switches
	} else {	// Plotter lab
	    DDRD |= clkX | clkY | dirX | dirY;
	    DDRB |= enX | sol | LED;
	    DDRB &= ~(limYA | limYD);
	    PORTB |= limYA | limYD;
	}


	PORTD &= ~(clkX | clkY | dirX | dirY);
	PORTB &= ~sol;
	PORTB &= ~enX;
	PORTB &= ~LED;


	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
	ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);
  ledON(); _delay_ms(50); ledOFF(); _delay_ms(50);



	home();

  solD();
  cuadrado(600);
  _delay_ms(500);
  D(4000);
  cruz(600);
  triangulo(5000);
  circulo();

  solU();



	return 0;
}
