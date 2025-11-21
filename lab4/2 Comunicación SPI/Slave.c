#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{

    DDRD |= (1<<PD3)|(1<<PD5)|(1<<PD6);   // D3 down, D5 right, D6 up
    DDRB |= (1<<PB1);                      // D9 left

    // PWM init
    TCCR0A = (1<<COM0A1)|(1<<COM0B1)|(1<<WGM00)|(1<<WGM01);
    TCCR0B = (1<<CS01);
    TCCR1A = (1<<COM1A1)|(1<<WGM10);
    TCCR1B = (1<<WGM12)|(1<<CS11);
    TCCR2A = (1<<COM2B1)|(1<<WGM20)|(1<<WGM21);
    TCCR2B = (1<<CS21);

    // SPI slave
    DDRB |= (1<<DDB4);
    SPCR = (1<<SPE);

    uint8_t x, y, dummy1, dummy2, dummy3;

    while (1)
{
    while (!(SPSR & (1<<SPIF))); uint8_t x      = SPDR;
    while (!(SPSR & (1<<SPIF))); uint8_t y      = SPDR;
    while (!(SPSR & (1<<SPIF))); uint8_t dummy1 = SPDR;
    while (!(SPSR & (1<<SPIF))); uint8_t dummy2 = SPDR;
    while (!(SPSR & (1<<SPIF))); uint8_t dummy3 = SPDR;
    while (!(SPSR & (1<<SPIF))); uint8_t fire   = SPDR;

    int8_t dx = (int8_t)x - 128;
    int8_t dy = (int8_t)y - 128;

    OCR1A = (dx < 0) ? (-dx * 2) : 0;   // D9  LEFT
    OCR0A = (dy < 0) ? (-dy * 2) : 0;   // D6  UP
    OCR0B = (dx > 0) ? ( dx * 2) : 0;   // D5  RIGHT
    OCR2B = (dy > 0) ? ( dy * 2) : 0;   // D3  DOWN

    // D4 = LED
    if (fire == 0xFF) PORTD |=  (1<<PD4);
    else              PORTD &= ~(1<<PD4);
}

}
