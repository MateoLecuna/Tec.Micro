#define F_CPU 8000000UL
#include <avr/io.h>

int main(void)
{
    DDRD |= (1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6);
    DDRB |= (1<<PB1);

    TCCR0A = 0b10100011; TCCR0B = 0b00000010;
    TCCR1A = 0b01000000; TCCR1B = 0b00001010;
    TCCR2A = 0b10100000; TCCR2B = 0b00000010;

    TWAR = 0x08 << 1;
    TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWINT);

    static uint8_t buffer[5] = {128,128,0,0,0};
    uint8_t idx = 0;

    while(1)
    {
        if (TWCR & (1<<TWINT))
        {
            buffer[idx] = TWDR;
            idx = (idx + 1) % 5;                              // buffer de 5

            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);        // siempre ACK
        }

        // Siempre tomamos los 3 últimos bytes REALES que envió el master
        // (aunque lleguen 5 eventos, los últimos 3 serán siempre X, Y, fire)
        uint8_t x    = buffer[(idx + 2) % 5];   // -3
        uint8_t y    = buffer[(idx + 3) % 5];   // -2
        uint8_t fire = buffer[(idx + 4) % 5];   // -1

        int8_t dx = (int8_t)x - 128;
        int8_t dy = (int8_t)y - 128;

        OCR1A = dx<0 ? -dx*2 : 0;
        OCR0A = dy<0 ? -dy*2 : 0;
        OCR0B = dx>0 ?  dx*2 : 0;
        OCR2B = dy>0 ?  dy*2 : 0;

        fire ? (PORTD |= (1<<PD4)) : (PORTD &= ~(1<<PD4));
    }
}