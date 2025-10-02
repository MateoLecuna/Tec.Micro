#include "draw.h"
#include "gpio.h"
#include "scheduler.h"

static int16_t cur_x=0, cur_y=0;

void draw_init(void){
	cur_x = 0; cur_y = 0;
	pen_up();
}

void set_pen(bool down){
	if (down) pen_down();
	else pen_up();
}

// Próximo paso: implementar Bresenham line (no bloqueante con colas) y Midpoint circle.
void draw_line(coord_t x0, coord_t y0, coord_t x1, coord_t y1){
	(void)x0; (void)y0; (void)x1; (void)y1;
	// TODO
}

void draw_circle(coord_t cx, coord_t cy, coord_t r){
	(void)cx; (void)cy; (void)r;
	// TODO
}
