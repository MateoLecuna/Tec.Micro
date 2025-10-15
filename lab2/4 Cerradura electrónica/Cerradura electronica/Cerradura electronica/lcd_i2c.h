#ifndef LCD_I2C_H
#define LCD_I2C_H
#include <stdint.h>
#include <stdbool.h>

void lcd_init(uint8_t addr7);
void lcd_backlight(bool on);
void lcd_clear(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char* s);
void lcd_print_char(char c);

#endif
