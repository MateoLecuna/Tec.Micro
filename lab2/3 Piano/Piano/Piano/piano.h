#ifndef PIANO_H
#define PIANO_H

#include <stdint.h>
#include <stdbool.h>

// Inicializa las 12 teclas con pull-up interno
void piano_init(void);

// Escanea teclas y reproduce su nota
void piano_mode_tick(void);

#endif
