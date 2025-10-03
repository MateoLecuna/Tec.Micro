#ifndef FIGURAS_H
#define FIGURAS_H

#include <stdint.h>

//Tempo_base es la duración de una negra en ms
extern uint16_t tempo_base;

//Macros en función de la variable
#define REDONDA			(4 * tempo_base)
#define BLANCA			(2 * tempo_base)
#define NEGRA			(1 * tempo_base)
#define CORCHEA			(tempo_base / 2)
#define SEMICORCHEA		(tempo_base / 4)
#define SEMISEMICORCHEA	(tempo_base / 8)

#endif
