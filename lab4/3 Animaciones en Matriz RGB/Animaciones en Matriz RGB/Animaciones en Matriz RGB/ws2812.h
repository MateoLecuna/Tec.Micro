#ifndef WS2812_H
#define WS2812_H

#include <stdint.h>
#include <avr/io.h>

#define WS2812_PORT PORTB
#define WS2812_DDR  DDRB
#define WS2812_PIN  PB0   // DIN de la matriz

void ws2812_init(void);
void ws2812_show(uint8_t *fb);

#endif
