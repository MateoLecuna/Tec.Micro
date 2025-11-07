#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "UART.h"

/* =================== CONFIGURACIÓN UART =================== */
#define UART_BAUD 9600UL
#define UBRR_VAL ((F_CPU/16/UART_BAUD)-1)

/* =================== WS2812B DRIVER =================== */
#define WS_PORT PORTB
#define WS_DDR  DDRB
#define WS_PIN  PB0

#define CYCLES(n) __builtin_avr_delay_cycles(n)

static inline void ws2812_send_bit(uint8_t one){
	if (one){
		WS_PORT |= (1<<WS_PIN); CYCLES(11);
		WS_PORT &= ~(1<<WS_PIN); CYCLES(9);
		} else {
		WS_PORT |= (1<<WS_PIN); CYCLES(4);
		WS_PORT &= ~(1<<WS_PIN); CYCLES(16);
	}
}
static inline void ws2812_send_byte(uint8_t b){
	for(uint8_t i=0;i<8;i++){ ws2812_send_bit(b & 0x80); b<<=1; }
}
static inline void ws2812_send_array(const uint8_t *grb, uint16_t n){
	cli();
	for(uint16_t i=0;i<n;i++){
		ws2812_send_byte(grb[3*i+0]); // G
		ws2812_send_byte(grb[3*i+1]); // R
		ws2812_send_byte(grb[3*i+2]); // B
	}
	sei();
	_delay_us(80);
}

/* =================== CONFIG MATRIZ =================== */
#define NUM_LEDS 64
static uint8_t leds[NUM_LEDS*3];
static inline uint8_t scale8(uint8_t v, uint8_t s){ return (uint16_t)v*s/255; }
static uint8_t BR = 80;

static inline void clear_leds(void){
	for(uint16_t i=0;i<NUM_LEDS*3;i++) leds[i]=0;
}
static inline void set_led(uint8_t idx,uint8_t r,uint8_t g,uint8_t b){
	if(idx>=NUM_LEDS) return;
	leds[3*idx+0]=scale8(g,BR);
	leds[3*idx+1]=scale8(r,BR);
	leds[3*idx+2]=scale8(b,BR);
}

/* =================== JOYSTICK =================== */
static void adc_init(void){
	ADMUX  = (1<<REFS0);  // AVcc ref
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1); // habilitar, prescaler 64
}
static uint16_t adc_read(uint8_t ch){
	ADMUX = (ADMUX & 0xF0) | (ch & 0x0F);
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	return ADC;
}

/* =================== BOTÓN =================== */
static inline void button_init(void){
	DDRD  &= ~(1<<PD2);
	PORTD |=  (1<<PD2);  // pull-up
}
static inline uint8_t button_pressed(void){
	return (PIND & (1<<PD2)) ? 0 : 1; // activo en bajo
}

/* =================== COLOR ALEATORIO =================== */
static uint8_t lfsr=0xA5;
static inline uint8_t rand8(void){
	uint8_t lsb=lfsr&1;
	lfsr>>=1;
	if(lsb) lfsr^=0xB8;
	return lfsr;
}
static inline void random_color(uint8_t *r,uint8_t *g,uint8_t *b){
	*r = rand8();
	*g = rand8();
	*b = rand8();
}

/* =================== MAIN =================== */
int main(void){
	WS_DDR |= (1<<WS_PIN);
	WS_PORT &= ~(1<<WS_PIN);

	uart_init(UBRR_VAL);
	adc_init();
	button_init();

	uint8_t i = 0; // LED actual
	uint8_t r=255,g=0,b=0;
	uint8_t btn_prev=0;
	const uint16_t DZ=90, MID=512;
	const uint16_t TH_HI=MID+DZ, TH_LO=MID-DZ;
	const uint8_t STEP_COOLDOWN=100/10;
	uint8_t cooldown=0;
	char buffer[50];

	while(1){
		uint16_t x=adc_read(1); // eje X
		uint16_t y=adc_read(0); // eje Y
		uint8_t btn=button_pressed();

		// Cambio de color al presionar
		if(btn && !btn_prev){
			random_color(&r,&g,&b);
			sprintf(buffer, "Nuevo color -> R=%3d, G=%3d, B=%3d\r\n", r, g, b);
			uart_print(buffer);
		}
		btn_prev=btn;

		if(cooldown>0) cooldown--;
		else{
			// === Movimiento X (invertido) ===
			if(x>TH_HI){ // IZQUIERDA (antes derecha)
				if(i!=0 && i!=8 && i!=16 && i!=24 && i!=32 && i!=40 && i!=48 && i!=56) i--;
				cooldown=STEP_COOLDOWN;
			}
			else if(x<TH_LO){ // DERECHA (antes izquierda)
				if(i!=7 && i!=15 && i!=23 && i!=31 && i!=39 && i!=47 && i!=55 && i!=63) i++;
				cooldown=STEP_COOLDOWN;
			}

			// === Movimiento Y ===
			if(y>TH_HI){ // abajo
				if(i+8<=63) i+=8;
				cooldown=STEP_COOLDOWN;
			}
			else if(y<TH_LO){ // arriba
				if(i>=8) i-=8;
				cooldown=STEP_COOLDOWN;
			}
		}

		clear_leds();
		set_led(i,r,g,b);
		ws2812_send_array(leds,NUM_LEDS);
		_delay_ms(10);
	}
}
