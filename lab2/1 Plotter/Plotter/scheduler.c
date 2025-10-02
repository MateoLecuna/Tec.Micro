#include "scheduler.h"
#include "gpio.h"

static volatile uint16_t ticks_left = 0;
static volatile move_t current = MV_NONE;
static volatile uint8_t diag_phase = 0; // para alternar ejes en diagonales

void sched_init(void){
	ticks_left = 0;
	current = MV_NONE;
	move_none();
	pen_up(); // arranque seguro
}

void sched_start(move_t mv, uint16_t duration_ms){
	current = mv;
	ticks_left = duration_ms;
	diag_phase = 0;

	switch(mv){
		case MV_UP:    move_up(); break;
		case MV_DOWN:  move_down(); break;
		case MV_LEFT:  move_left(); break;
		case MV_RIGHT: move_right(); break;
		case MV_DIAG_UR:
		case MV_DIAG_UL:
		case MV_DIAG_DR:
		case MV_DIAG_DL:
		// arrancamos con un eje; iremos alternando en tick
		// arrancar “none”; tick decide eje cada ms
		move_none();
		break;
		default:
		move_none(); break;
	}
}

bool sched_busy(void){
	return (ticks_left != 0);
}

void sched_stop(void){
	ticks_left = 0;
	current = MV_NONE;
	move_none();
}

void sched_tick_isr(void){
	if (!ticks_left) return;

	// Manejo diagonal alternado: eje X e Y en t alternos
	if (current==MV_DIAG_UR || current==MV_DIAG_UL ||
	current==MV_DIAG_DR || current==MV_DIAG_DL){
		diag_phase ^= 1;
		if (diag_phase==0){
			// eje X
			if (current==MV_DIAG_UR || current==MV_DIAG_DR) move_right();
			else move_left();
			}else{
			// eje Y
			if (current==MV_DIAG_UR || current==MV_DIAG_UL) move_up();
			else move_down();
		}
	}

	if (--ticks_left == 0){
		move_none();
		current = MV_NONE;
	}
}
