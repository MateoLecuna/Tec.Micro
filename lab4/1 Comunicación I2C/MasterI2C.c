#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#define LCD_I2C_ADDR  0x27
#define SLAVE_ADDR    0x08
#define LCD_BACKLIGHT 0x08
#define LCD_EN        0b00000100
#define LCD_RS        0b00000001
#define BUTTON_PIN    PD4

char buf[30];

// -------------------- JOYSTICK --------------------
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

// -------------------- I2C MASTER --------------------
void I2C_Init(void)   { TWSR = 0; TWBR = 72; TWCR = (1<<TWEN); }
void I2C_Start(void)  { TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); while(!(TWCR&(1<<TWINT))); }
void I2C_Stop(void)   { TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN); }
void I2C_Write(uint8_t d) { TWDR = d; TWCR = (1<<TWINT)|(1<<TWEN); while(!(TWCR&(1<<TWINT))); }

void send_to_slave(uint8_t x, uint8_t y, uint8_t fire) {
    I2C_Start();
    I2C_Write(SLAVE_ADDR << 1);
    I2C_Write(x);
    I2C_Write(y);
    I2C_Write(fire);
    I2C_Stop();
}

// -------------------- LCD --------------------
void LCD_SendNibble(uint8_t n, uint8_t mode) {
    uint8_t data = n | mode | LCD_BACKLIGHT;
    I2C_Start(); I2C_Write(LCD_I2C_ADDR<<1);
    I2C_Write(data | LCD_EN); _delay_us(1);
    I2C_Write(data & ~LCD_EN); _delay_us(50);
    I2C_Stop();
}
void LCD_SendByte(uint8_t b, uint8_t mode) { LCD_SendNibble(b & 0xF0, mode); LCD_SendNibble(b << 4, mode); }
void LCD_Command(uint8_t c)  { LCD_SendByte(c, 0); }
void LCD_Data(uint8_t d)     { LCD_SendByte(d, LCD_RS); }
void LCD_Init(void) {
    _delay_ms(50);
    LCD_SendNibble(0x30,0); _delay_ms(5);
    LCD_SendNibble(0x30,0); _delay_us(200);
    LCD_SendNibble(0x30,0);
    LCD_SendNibble(0x20,0);
    LCD_Command(0x28); LCD_Command(0x08); LCD_Command(0x01);
    _delay_ms(3); LCD_Command(0x06); LCD_Command(0x0C);
}
void LCD_Print(char *s)      { while(*s) LCD_Data(*s++); }
void LCD_SetCursor(uint8_t r, uint8_t c) { LCD_Command(0x80 | (c + (r?0x40:0))); }

// -------------------- ADC --------------------
void ADC_Init(void)   { ADMUX = (1<<REFS0); ADCSRA = (1<<ADEN)|(7); }
uint16_t ADC_Read(uint8_t ch) {
    ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
    ADCSRA |= (1<<ADSC);
    while(ADCSRA & (1<<ADSC));
    return ADC;
}

// -------------------- MAIN --------------------
int main(void) {
    I2C_Init();
    LCD_Init();
    ADC_Init();

    DDRD &= ~(1<<BUTTON_PIN);
    PORTD |= (1<<BUTTON_PIN);

    LCD_SetCursor(0,0); LCD_Print("I2C CROSS + LASER  ");
    LCD_SetCursor(1,0); LCD_Print("READY              ");

    while(1) {
        uint8_t x    = read_joy_x();
        uint8_t y    = read_joy_y();
        uint8_t fire = (PIND & (1<<BUTTON_PIN)) ? 0 : 255;

        send_to_slave(x, y, fire);

        static uint16_t cnt = 0;
        if(++cnt >= 25) {
            cnt = 0;
            LCD_SetCursor(0,2);
            sprintf(buf,"X:%3d Y:%3d F:%s", x, y, fire?"ON ":"OFF");
            LCD_Print(buf);
        }
        _delay_ms(5);
    }
}