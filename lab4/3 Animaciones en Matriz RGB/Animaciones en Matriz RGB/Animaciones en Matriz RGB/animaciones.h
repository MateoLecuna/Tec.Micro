#ifndef ANIMACIONES_H
#define ANIMACIONES_H

#include <stdint.h>

#define MODO_OFF   0
#define MODO_ANIM1 1
#define MODO_ANIM2 2

void animaciones_init(uint8_t *fb);
void animaciones_set_modo(uint8_t modo, uint8_t *fb);

// Devuelve 1 si el frame_buffer fue modificado (hay que refrescar la matriz)
uint8_t animaciones_update(uint32_t t_ms, uint8_t *fb);

#endif
