#include "hardware.h"
#include <avr/io.h>
#include <util/delay.h>
#include "twi.h"
#include "lcd_i2c.h"

static void leds_iniciar(void) {
	DDRD |= (1<<PIN_LED_VERDE_PD) | (1<<PIN_LED_ROJO_PD);
	PORTD &= ~((1<<PIN_LED_VERDE_PD) | (1<<PIN_LED_ROJO_PD));
}

void leds(bool verde, bool rojo) {
	if (verde) PORTD |=  (1<<PIN_LED_VERDE_PD); else PORTD &= ~(1<<PIN_LED_VERDE_PD);
	if (rojo)  PORTD |=  (1<<PIN_LED_ROJO_PD);  else PORTD &= ~(1<<PIN_LED_ROJO_PD);
}

void hardware_iniciar(void) {
	// LEDs
	leds_iniciar();

	// I2C a 100 kHz
	twi_init(100000UL);

	// LCD
	lcd_init(LCD_I2C_ADDR);
	lcd_backlight(true);
}

void lcd_titulo(const char* fila0, const char* fila1) {
	lcd_clear();
	lcd_set_cursor(0,0);
	lcd_print(fila0);
	lcd_set_cursor(0,1);
	lcd_print(fila1);
}

void lcd_limpiar_fila(uint8_t fila) {
	uint8_t i;
	lcd_set_cursor(0, fila);
	for (i=0;i<16;i++) lcd_print_char(' ');
	lcd_set_cursor(0, fila);
}
