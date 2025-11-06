#define F_CPU 16000000L

#include <avr/io.h>
#include <util/delay.h>


int main (void){
	DDRD = 0x20;
	PORTD = 0;
	
	while(1){
		if (PIND & 0x04){
			PORTD = 0x20;
		} else {
			PORTD = 0;
		}
	}
}
