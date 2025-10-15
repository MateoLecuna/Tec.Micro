#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <avr/io.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

void uart_init(uint32_t baud);
void uart_putchar(char c);
void uart_write(const char *s);
uint8_t uart_available(void);
int16_t uart_getchar(void);              // -1 si vac�o
int8_t uart_readline(char *dst, uint8_t maxlen); // devuelve n bytes (sin '\n'), 0 si no hay l�nea

#endif
