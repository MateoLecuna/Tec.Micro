#ifndef GPIO_H
#define GPIO_H

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

void gpio_init(void);

// Solenoide (punta)
void pen_down(void);
void pen_up(void);

// Movimiento cardinal (mantener dirección activa mientras dure el tramo)
void move_none(void);
void move_up(void);
void move_down(void);
void move_left(void);
void move_right(void);

// Diagonal simple (alterna ejes desde el scheduler)
void move_diag(int8_t dx, int8_t dy);

#endif
