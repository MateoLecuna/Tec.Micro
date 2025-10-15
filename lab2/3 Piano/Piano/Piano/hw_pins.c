#include "hw_pins.h"

void hw_init_pins(void) {
    // Audio pin como salida, inicialmente en 0 
    AUDIO_DDR  |=  (1<<AUDIO_PIN);
    AUDIO_PORT &= ~(1<<AUDIO_PIN);

    // Notas: entradas con pull-up
    NOTE_PORTC_DDR  &= ~NOTES_MASK_C;
    NOTE_PORTC_PORT |=  NOTES_MASK_C;

    NOTE_PORTD_DDR  &= ~NOTES_MASK_D;
    NOTE_PORTD_PORT |=  NOTES_MASK_D;
}

uint16_t hw_read_notes_raw(void) {
    uint16_t bits = 0;

    // PC0..PC5 --> bits 0..5
    uint8_t c = NOTE_PORTC_PIN & (uint8_t)NOTES_MASK_C;
	
    // Compactar PC0..PC5 a bits 0..5 directamente
    bits |= (c & (1<<PC0)) ? (1<<0) : 0;
    bits |= (c & (1<<PC1)) ? (1<<1) : 0;
    bits |= (c & (1<<PC2)) ? (1<<2) : 0;
    bits |= (c & (1<<PC3)) ? (1<<3) : 0;
    bits |= (c & (1<<PC4)) ? (1<<4) : 0;
    bits |= (c & (1<<PC5)) ? (1<<5) : 0;

    // PD2..PD7 -> bits 6..11 
    uint8_t d = NOTE_PORTD_PIN & (uint8_t)NOTES_MASK_D;
    bits |= (d & (1<<PD2)) ? (1<<6)  : 0;
    bits |= (d & (1<<PD3)) ? (1<<7)  : 0;
    bits |= (d & (1<<PD4)) ? (1<<8)  : 0;
    bits |= (d & (1<<PD5)) ? (1<<9)  : 0;
    bits |= (d & (1<<PD6)) ? (1<<10) : 0;
    bits |= (d & (1<<PD7)) ? (1<<11) : 0;

    return bits; // 1=libre, 0=presionado (por pull-ups)
}
