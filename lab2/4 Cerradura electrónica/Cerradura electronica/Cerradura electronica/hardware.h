#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>
#include <stdbool.h>

#define F_CPU 16000000UL

// LEDs en PORTD
#define PIN_LED_VERDE_PD   PD2
#define PIN_LED_ROJO_PD    PD3

// Teclado: filas PB0..PB3 (salida), columnas PB4..PB7 (entrada con pull-up)
#define TECLADO_FILA0_PB   PB0
#define TECLADO_FILA1_PB   PB1
#define TECLADO_FILA2_PB   PB2
#define TECLADO_FILA3_PB   PB3
#define TECLADO_COL0_PB    PB4
#define TECLADO_COL1_PB    PB5
#define TECLADO_COL2_PB    PB6
#define TECLADO_COL3_PB    PB7

// LCD I2C
#define LCD_I2C_ADDR       0x27  // 0x27 o 0x3F según tu backpack

void hardware_iniciar(void);
void leds(bool verde, bool rojo);

// helpers de LCD
void lcd_titulo(const char* fila0, const char* fila1);
void lcd_limpiar_fila(uint8_t fila);

#endif
