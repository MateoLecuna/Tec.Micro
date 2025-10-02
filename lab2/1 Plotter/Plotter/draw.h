#ifndef DRAW_H
#define DRAW_H
#include <stdint.h>
#include <stdbool.h>

// Coordenadas en “pasos”
typedef int16_t coord_t;

void draw_init(void);

// Futuro: versiones no bloqueantes con colas internas
void draw_line(coord_t x0, coord_t y0, coord_t x1, coord_t y1);
void draw_circle(coord_t cx, coord_t cy, coord_t r);

// Helpers
void set_pen(bool down);

#endif
