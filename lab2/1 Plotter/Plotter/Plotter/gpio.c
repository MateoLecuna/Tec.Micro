#include "gpio.h"
#include "config.h"
#include <util/atomic.h>

static inline void set_bit(volatile uint8_t* port, uint8_t bit){ *port |=  (1<<bit); }
static inline void clr_bit(volatile uint8_t* port, uint8_t bit){ *port &= ~(1<<bit); }

void gpio_init(void) {
	// PD2..PD7 salidas
	DDRD |= (1<<PIN_SOL_DOWN) | (1<<PIN_SOL_UP) |
	(1<<PIN_MOVE_D)  | (1<<PIN_MOVE_U) |
	(1<<PIN_MOVE_R)  | (1<<PIN_MOVE_L);

	// Arranque seguro
	PORTD &= ~((1<<PIN_SOL_DOWN)|(1<<PIN_SOL_UP)|
	(1<<PIN_MOVE_D)|(1<<PIN_MOVE_U)|
	(1<<PIN_MOVE_R)|(1<<PIN_MOVE_L));
}

void pen_down(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		// Nunca energizar ambos a la vez
		clr_bit(&PORTD, PIN_SOL_UP);
		set_bit(&PORTD, PIN_SOL_DOWN);
	}
}

void pen_up(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		clr_bit(&PORTD, PIN_SOL_DOWN);
		set_bit(&PORTD, PIN_SOL_UP);
	}
}

void move_none(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		clr_bit(&PORTD, PIN_MOVE_U);
		clr_bit(&PORTD, PIN_MOVE_D);
		clr_bit(&PORTD, PIN_MOVE_L);
		clr_bit(&PORTD, PIN_MOVE_R);
	}
}

void move_up(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		set_bit(&PORTD, PIN_MOVE_U);
		clr_bit(&PORTD, PIN_MOVE_D);
		clr_bit(&PORTD, PIN_MOVE_L);
		clr_bit(&PORTD, PIN_MOVE_R);
	}
}

void move_down(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		set_bit(&PORTD, PIN_MOVE_D);
		clr_bit(&PORTD, PIN_MOVE_U);
		clr_bit(&PORTD, PIN_MOVE_L);
		clr_bit(&PORTD, PIN_MOVE_R);
	}
}

void move_left(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		set_bit(&PORTD, PIN_MOVE_L);
		clr_bit(&PORTD, PIN_MOVE_R);
		clr_bit(&PORTD, PIN_MOVE_U);
		clr_bit(&PORTD, PIN_MOVE_D);
	}
}

void move_right(void) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		set_bit(&PORTD, PIN_MOVE_R);
		clr_bit(&PORTD, PIN_MOVE_L);
		clr_bit(&PORTD, PIN_MOVE_U);
		clr_bit(&PORTD, PIN_MOVE_D);
	}
}

// Diagonal “simbólica”: el scheduler alternará ejes para que sea pareja
void move_diag(int8_t dx, int8_t dy) {
	if (dx > 0) move_right();
	else if (dx < 0) move_left();
	if (dy > 0) move_up();
	else if (dy < 0) move_down();
	// Nota: el alternado real lo hace el scheduler (ticks pares/impares)
}
