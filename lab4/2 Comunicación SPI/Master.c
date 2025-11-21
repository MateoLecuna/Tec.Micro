#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#define LCD_I2C_ADDR  0x27
#define LCD_BACKLIGHT 0x08
#define LCD_EN        0b00000100
#define LCD_RS        0b00000001
#define BUTTON_PIN    PD4

char buf[30];

uint8_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint8_t out_min, uint8_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
uint8_t read_joy_x(void) {
    uint16_t raw = ADC_Read(0);
    if (raw > 480 && raw < 550) return 128;
    return (raw > 512) ? map(raw, 550, 1023, 128, 255) : map(raw, 0, 480, 0, 128);
}
uint8_t read_joy_y(void) {
    uint16_t raw = ADC_Read(1);
    if (raw > 480 && raw < 550) return 128;
    return (raw > 512) ? map(raw, 550, 1023, 128, 255) : map(raw, 0, 480, 0, 128);
}

uint8_t read_temperature(void) { return 23 + (rand() % 15); }
uint8_t read_humidity(void)    { return 40 + (rand() % 40); }
uint8_t read_light(void)       { return ADC_Read(2) >> 2; }

void I2C_Init(void) { TWSR = 0; TWBR = ((F_CPU / 100000UL) - 16) / 2; TWCR = (1 << TWEN); }
void I2C_Start(void)  { TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); while (!(TWCR & (1 << TWINT))); }
void I2C_Stop(void)   { TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); _delay_us(100); }
void I2C_Write(uint8_t d) { TWDR = d; TWCR = (1 << TWINT) | (1 << TWEN); while (!(TWCR & (1 << TWINT))); }
void LCD_SendNibble(uint8_t nibble, uint8_t mode) {
    uint8_t data = nibble | mode | LCD_BACKLIGHT;
    I2C_Start(); I2C_Write(LCD_I2C_ADDR << 1);
    I2C_Write(data | LCD_EN); _delay_us(1);
    I2C_Write(data & ~LCD_EN); _delay_us(50);
    I2C_Stop();
}
void LCD_SendByte(uint8_t byte, uint8_t mode) { LCD_SendNibble(byte & 0xF0, mode); LCD_SendNibble(byte << 4, mode); _delay_us(50); }
void LCD_Command(uint8_t cmd) { LCD_SendByte(cmd, 0); }
void LCD_Data(uint8_t data)   { LCD_SendByte(data, LCD_RS); }
void LCD_Init(void) {
    _delay_ms(50);
    LCD_SendNibble(0x30, 0); _delay_ms(5);
    LCD_SendNibble(0x30, 0); _delay_us(200);
    LCD_SendNibble(0x30, 0); _delay_us(200);
    LCD_SendNibble(0x20, 0); _delay_us(200);
    LCD_Command(0x28); LCD_Command(0x08); LCD_Command(0x01);
    _delay_ms(3); LCD_Command(0x06); LCD_Command(0x0C);
}
void LCD_Print(char *s) { while (*s) LCD_Data(*s++); }
void LCD_SetCursor(uint8_t row, uint8_t col) { LCD_Command(0x80 | (col + (row ? 0x40 : 0x00))); }

void SPI_Master_Init(void) {
    DDRB |= (1<<DDB3)|(1<<DDB5)|(1<<DDB2);
    DDRB &= ~(1<<DDB4);
    PORTB |= (1<<PB2);
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}
uint8_t SPI_Transfer(uint8_t d) {
    PORTB &= ~(1<<PB2); _delay_us(10);
    SPDR = d; while(!(SPSR & (1<<SPIF)));
    uint8_t r = SPDR;
    PORTB |= (1<<PB2);
    return r;
}

void ADC_Init(void) { ADMUX = (1<<REFS0); ADCSRA = (1<<ADEN) | (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); }
uint16_t ADC_Read(uint8_t ch) {
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
    ADCSRA |= (1<<ADSC);
    while (ADCSRA & (1<<ADSC));
    return ADC;
}

// MAIN — FINAL
int main(void) {
    I2C_Init(); LCD_Init(); SPI_Master_Init(); ADC_Init();
    DDRD &= ~(1<<BUTTON_PIN); PORTD |= (1<<BUTTON_PIN);

    LCD_SetCursor(0,0); LCD_Print("X:     Y:     F:OFF");
    LCD_SetCursor(1,0); LCD_Print("CROSS + LASER READY ");

    uint8_t x, y, fire;

    while (1) {
        x = read_joy_x();
        y = read_joy_y();
        fire = (PIND & (1<<BUTTON_PIN)) ? 0x00 : 0xFF;

        SPI_Transfer(x);
        SPI_Transfer(y);
        SPI_Transfer(fire);

        static uint16_t cnt = 0;
        if (++cnt >= 25) {
            cnt = 0;
            LCD_SetCursor(0,2); sprintf(buf,"%3d %3d F:%s", x, y, fire?"ON ":"OFF"); LCD_Print(buf);
        }
        _delay_ms(5);
    }

}
