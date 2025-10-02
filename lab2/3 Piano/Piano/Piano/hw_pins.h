#ifndef HW_PINS_H
#define HW_PINS_H

#include <avr/io.h>

/* 
	===== Salida de audio =====
	Usamos OC2A = PB3 (pin D11 en Arduino UNO)
*/
#define AUDIO_DDR   DDRB
#define AUDIO_PORT  PORTB
#define AUDIO_PIN   PB3

/* 
	===== Entradas de notas (12 teclas) =====
	6 en PORTC: PC0..PC5  (C, C#, D, D#, E, F)
	6 en PORTD: PD2..PD7  (F#, G, G#, A, A#, B)
	*Todas con pull-up internas; activo en bajo (0 = presionado)
*/
#define NOTE_PORTC_PIN   PINC
#define NOTE_PORTC_DDR   DDRC
#define NOTE_PORTC_PORT  PORTC

#define NOTE_PORTD_PIN   PIND
#define NOTE_PORTD_DDR   DDRD
#define NOTE_PORTD_PORT  PORTD

/* Máscaras */
#define NOTES_MASK_C   ((1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3)|(1<<PC4)|(1<<PC5))
#define NOTES_MASK_D   ((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6)|(1<<PD7))

/* Índices sugeridos de 0..11 (cromático: C, C#, D, D#, E, F, F#, G, G#, A, A#, B) */
enum {
    N_C = 0, N_Cs, N_D, N_Ds, N_E, N_F, N_Fs, N_G, N_Gs, N_A, N_As, N_B, N_COUNT
};

/* 
	===== Botones de Octava +/- =====
	Usamos PB0 = OCT_DOWN, PB1 = OCT_UP (con pull-up; activo en bajo)
	(Dejamos PD0/PD1 libres para UART)
*/
#define OCT_DDR   DDRB
#define OCT_PORT  PORTB
#define OCT_PINR  PINB
#define OCT_DOWN  PB0
#define OCT_UP    PB1
#define OCT_MASK  ((1<<OCT_DOWN)|(1<<OCT_UP))

/* Funciones públicas */
void hw_init_pins(void);
uint16_t hw_read_notes_raw(void);   // bit0..bit11 (1=libre, 0=presionado invertiremos después)
uint8_t  hw_read_oct_buttons(void); // bit0=DOWN, bit1=UP (1=libre, 0=presionado)

#endif
