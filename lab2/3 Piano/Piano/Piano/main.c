#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "hw_pins.h"
#include "sound.h"

int main(void) {
    hw_init_pins();
    sound_init_idle();

    while (1) {
        uint16_t notes = hw_read_notes_raw(); // 1=libre, 0=presionado
        uint8_t  oct   = hw_read_oct_buttons(); // 1=libre, 0=presionado

        (void)notes;
        (void)oct;

        _delay_ms(10);
    }
}

