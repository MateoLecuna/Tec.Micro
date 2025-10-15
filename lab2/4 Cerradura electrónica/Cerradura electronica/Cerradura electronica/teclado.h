#ifndef TECLADO_H
#define TECLADO_H
#include <stdint.h>
#include <stdbool.h>

// Devuelve '\0' si no hay tecla, o el caracter ('0' - '9','A' - 'D','*','#')
char teclado_leer(void);
void teclado_iniciar(void);

#endif
