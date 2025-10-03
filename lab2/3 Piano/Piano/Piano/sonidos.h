#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

void sound_init_timer2(void);          // Configura Timer2 (CTC toggle OC2A, presc=128)
void sound_off(void);                  // Silencio (OC2A deshabilitado)
void sound_note_on(uint8_t idx, int8_t octave); // idx: 0..11 (C..B), octave: -2..+2 (p.ej.)

#endif
