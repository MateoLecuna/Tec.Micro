#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

// LCD: Pines en PORTD
#define LCD_RS PD2
#define LCD_E  PD3
#define LCD_D4 PD4
#define LCD_D5 PD5
#define LCD_D6 PD6
#define LCD_D7 PD7

// Helpers de bit
#define SETBIT(port,bit)    ((port) |=  (1<<(bit)))
#define CLRBIT(port,bit)    ((port) &= ~(1<<(bit)))
#define READBIT(pin,bit)    ((pin)  &   (1<<(bit)))

// Prototipos LCD
static void lcd_pulse_enable(void);
static void lcd_write_nibble(uint8_t nib);
static void lcd_write_byte(uint8_t data, uint8_t is_data);
static void lcd_cmd(uint8_t cmd);
static void lcd_data(uint8_t data);
static void lcd_init(void);
static void lcd_gotoxy(uint8_t col, uint8_t row);
static void lcd_puts(const char *s);

// Teclado 4x4
// Filas -> PC0..PC3 (salidas), Columnas -> PB0..PB3 (entradas pull-up)
static void kpd_init(void);
static char kpd_getkey(void);     // devuelve '\0' si no hay tecla
static char keymap(uint8_t r, uint8_t c);

// Implementación LCD
static void lcd_pulse_enable(void) {
    SETBIT(PORTD, LCD_E);
    _delay_us(1);
    CLRBIT(PORTD, LCD_E);
    _delay_us(50);
}

static void lcd_write_nibble(uint8_t nib) {
    // Poner D4..D7 según nibble (bits 0..3)
    if (nib & 0x01) SETBIT(PORTD, LCD_D4); else CLRBIT(PORTD, LCD_D4);
    if (nib & 0x02) SETBIT(PORTD, LCD_D5); else CLRBIT(PORTD, LCD_D5);
    if (nib & 0x04) SETBIT(PORTD, LCD_D6); else CLRBIT(PORTD, LCD_D6);
    if (nib & 0x08) SETBIT(PORTD, LCD_D7); else CLRBIT(PORTD, LCD_D7);
    lcd_pulse_enable();
}

static void lcd_write_byte(uint8_t data, uint8_t is_data) {
    if (is_data) SETBIT(PORTD, LCD_RS); else CLRBIT(PORTD, LCD_RS);
    lcd_write_nibble(data >> 4);
    lcd_write_nibble(data & 0x0F);
}

static void lcd_cmd(uint8_t cmd) {
    lcd_write_byte(cmd, 0);
    if (cmd == 0x01 || cmd == 0x02) _delay_ms(2);
}

static void lcd_data(uint8_t data) {
    lcd_write_byte(data, 1);
}

static void lcd_init(void) {
    // Configurar pines como salida
    DDRD |= (1<<LCD_RS) | (1<<LCD_E) | (1<<LCD_D4) | (1<<LCD_D5) | (1<<LCD_D6) | (1<<LCD_D7);
    // Estado inicial
    CLRBIT(PORTD, LCD_RS);
    CLRBIT(PORTD, LCD_E);

    _delay_ms(40); // power-up

    // Init especial 4-bit
    lcd_write_nibble(0x03); _delay_ms(5);
    lcd_write_nibble(0x03); _delay_us(150);
    lcd_write_nibble(0x03); _delay_us(150);
    lcd_write_nibble(0x02); // 4-bit

    // Función: 4-bit, 2 líneas, 5x8
    lcd_cmd(0x28);
    // Display ON, cursor OFF, blink OFF
    lcd_cmd(0x0C);
    // Entry mode: cursor incrementa
    lcd_cmd(0x06);
    // Clear
    lcd_cmd(0x01);
}

static void lcd_gotoxy(uint8_t col, uint8_t row) {
    static const uint8_t base[2] = {0x00, 0x40}; // 16x2
    lcd_cmd(0x80 | (base[row & 1] + (col & 0x0F)));
}

static void lcd_puts(const char *s) {
    while (*s) lcd_data((uint8_t)*s++);
}

// Implementación Teclado
static void kpd_init(void) {
    // Filas PC0..PC3 como salidas (inician en alto)
    DDRC |= 0x0F;
    PORTC |= 0x0F;

    // Columnas PB0..PB3 como entradas con pull-up
    DDRB &= ~0x0F;
    PORTB |= 0x0F;
}

static char keymap(uint8_t r, uint8_t c) {
    static const char tbl[4][4] = {
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','C'},
        {'*','0','#','D'}
    };
    if (r<4 && c<4) return tbl[r][c];
    return '\0';
}

static char kpd_getkey(void) {
    // Escaneo por filas: poner una fila a 0, leer columnas
    for (uint8_t r = 0; r < 4; r++) {
        // Poner todas las filas en 1
        PORTC |= 0x0F;
        // Activar fila r en 0
        CLRBIT(PORTC, r);

        _delay_us(5);
		
        uint8_t cols = PINB & 0x0F; // columnas con pull-up
        if (cols != 0x0F) {
            // Alguna columna fue tirada a 0 -> hay tecla
            _delay_ms(20); // debounce
            cols = PINB & 0x0F;
            if (cols != 0x0F) {
                // Identificar columna
                for (uint8_t c = 0; c < 4; c++) {
                    if ((cols & (1<<c)) == 0) {
                        char ch = keymap(r, c);
                        // Esperar liberación para no repetir
                        while ((PINB & (1<<c)) == 0) { /* wait */ }
                        _delay_ms(10);
                        return ch;
                    }
                }
            }
        }
    }
    return '\0'; // sin tecla
}
	
int main(void) {
    lcd_init();
    kpd_init();
	
	lcd_gotoxy(0,0);
    lcd_puts("Tecla:");
    lcd_gotoxy(0,1);
    lcd_puts("");

    while (1) {
        char k = kpd_getkey();
        if (k != '\0') {
            lcd_data((uint8_t)k);   // muestra la tecla
            _delay_ms(350);         // visible 350 ms

            lcd_cmd(0x10);          // cursor a la izquierda
            lcd_data(' ');          // borra pisando con espacio
            lcd_cmd(0x10);          // vuelve a la posición original
        }
        _delay_ms(5);
    }
}