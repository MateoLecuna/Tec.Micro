#include "lcd_i2c.h"
#include "twi.h"
#include <util/delay.h>

static uint8_t g_addr;      // 7-bit address (e.g., 0x27)
static uint8_t g_bl = 0x08; // backlight bit (P3)

#define LCD_RS 0x01
#define LCD_RW 0x02
#define LCD_EN 0x04

// PCF8574 pins: P0=RS, P1=RW, P2=E, P3=BL, P4..P7=D4..D7

static void pcf_write(uint8_t data) {
	// SLA+W
	if (!twi_start((g_addr<<1) | 0)) return;
	twi_write(data | g_bl);
	twi_stop();
}

static void lcd_pulse(uint8_t data) {
	pcf_write(data | LCD_EN);
	_delay_us(1);
	pcf_write(data & ~LCD_EN);
	_delay_us(50);
}

static void lcd_write4(uint8_t nib_hi, uint8_t rs) {
	uint8_t d = (nib_hi & 0xF0) | (rs ? LCD_RS:0);
	lcd_pulse(d);
}

static void lcd_cmd(uint8_t cmd) {
	lcd_write4(cmd & 0xF0, 0);
	lcd_write4((cmd<<4) & 0xF0, 0);
	if (cmd == 0x01 || cmd == 0x02) _delay_ms(2);
}

static void lcd_dat(uint8_t dat) {
	lcd_write4(dat & 0xF0, 1);
	lcd_write4((dat<<4) & 0xF0, 1);
}

void lcd_init(uint8_t addr7) {
	g_addr = addr7;
	_delay_ms(40);
	// 3 veces 0x30 (modo 8-bit init sequence)
	lcd_write4(0x30,0); _delay_ms(5);
	lcd_write4(0x30,0); _delay_us(150);
	lcd_write4(0x30,0); _delay_us(150);
	// 0x20 -> 4-bit
	lcd_write4(0x20,0); _delay_us(150);

	// Función: 4-bit, 2 líneas, 5x8
	lcd_cmd(0x28);
	// Display ON, cursor off, blink off
	lcd_cmd(0x0C);
	// Clear
	lcd_cmd(0x01);
	// Entry mode: increment, no shift
	lcd_cmd(0x06);
}

void lcd_backlight(bool on) {
	g_bl = on ? 0x08 : 0x00;
	// Force a no-op write to latch BL
	pcf_write(0x00);
}

void lcd_clear(void) {
	lcd_cmd(0x01);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
	static const uint8_t base[2] = {0x00, 0x40};
	lcd_cmd(0x80 | (base[row%2] + (col%16)));
}

void lcd_print(const char* s) {
	while (*s) {
		lcd_dat((uint8_t)*s++);
	}
}

void lcd_print_char(char c) {
	lcd_dat((uint8_t)c);
}
