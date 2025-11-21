#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "UART.h"

#define UART_BAUD 9600UL
#define UBRR_VAL ((F_CPU/16/UART_BAUD)-1)

/* ================= WS2812B ================= */

#define WS_PORT PORTB
#define WS_DDR  DDRB
#define WS_PIN  PB0

#define CYCLES(n) __builtin_avr_delay_cycles(n)

static inline void ws2812_send_bit(uint8_t one){
	if(one){
		WS_PORT |= (1<<WS_PIN); CYCLES(11);
		WS_PORT &= ~(1<<WS_PIN); CYCLES(9);
		} else {
		WS_PORT |= (1<<WS_PIN); CYCLES(4);
		WS_PORT &= ~(1<<WS_PIN); CYCLES(16);
	}
}

static inline void ws2812_send_byte(uint8_t b){
	for(uint8_t i=0;i<8;i++){
		ws2812_send_bit(b & 0x80);
		b <<= 1;
	}
}

static inline void ws2812_send_array(const uint8_t *grb, uint16_t n){
	cli();
	for(uint16_t i=0;i<n;i++){
		ws2812_send_byte(grb[3*i+0]);
		ws2812_send_byte(grb[3*i+1]);
		ws2812_send_byte(grb[3*i+2]);
	}
	sei();
	_delay_us(80);
}

/* ================= MATRIZ ================= */

#define NUM_LEDS 64
static uint8_t leds[NUM_LEDS*3];

static inline uint8_t scale8(uint8_t v, uint8_t s){
	return (uint16_t)v * s / 255;
}

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

/* ================= BOTÓN ================= */

static inline void button_init(void){
	DDRD  &= ~(1<<PD2);
	PORTD |=  (1<<PD2);
}

static inline uint8_t button_pressed(void){
	return (PIND & (1<<PD2)) ? 0 : 1;
}

/* ================= I2C + MPU6050 ================= */

#define MPU6050_ADDR 0x68

void i2c_init(void){
	TWSR = 0;
	TWBR = 72;
}

void i2c_start(void){
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
}

void i2c_stop(void){
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

void i2c_write(uint8_t data){
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
}

uint8_t i2c_read_ack(void){
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while(!(TWCR & (1<<TWINT)));
	return TWDR;
}

uint8_t i2c_read_nack(void){
	TWCR = (1<<TWINT)|(1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	return TWDR;
}

void mpu6050_init(void){
	i2c_start();
	i2c_write((MPU6050_ADDR<<1) | 0);
	i2c_write(0x6B);
	i2c_write(0x00);
	i2c_stop();
}

void mpu6050_read_accel(int16_t *ax, int16_t *ay){
	i2c_start();
	i2c_write((MPU6050_ADDR<<1) | 0);
	i2c_write(0x3B);

	i2c_start();
	i2c_write((MPU6050_ADDR<<1) | 1);

	uint8_t axh = i2c_read_ack();
	uint8_t axl = i2c_read_ack();
	uint8_t ayh = i2c_read_ack();
	uint8_t ayl = i2c_read_nack();

	i2c_stop();

	*ax = (axh<<8) | axl;
	*ay = (ayh<<8) | ayl;
}

/* ================= COLOR ================= */

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

/* ================= MAIN ================= */

int main(void){
	WS_DDR |= (1<<WS_PIN);
	WS_PORT &= ~(1<<WS_PIN);

	uart_init(UBRR_VAL);
	button_init();
	i2c_init();
	_delay_ms(100);
	mpu6050_init();

	uint8_t i = 27;  // LED inicial (centro aprox)
	uint8_t r=255,g=0,b=0;
	uint8_t btn_prev=0;

	uint8_t cooldown=0;
	const uint8_t STEP_COOLDOWN=10;
	int16_t ax, ay;
	int16_t umbral = 6000;

	char buffer[50];
	uart_print("Sistema iniciado (MPU + Matriz)\r\n");

	while(1){
		uint8_t btn = button_pressed();

		if(btn && !btn_prev){
			random_color(&r,&g,&b);
			sprintf(buffer,"Color -> R=%d G=%d B=%d\r\n",r,g,b);
			uart_print(buffer);
		}
		btn_prev = btn;

		mpu6050_read_accel(&ax,&ay);
		ay = -ay;

		if(cooldown>0) cooldown--;
		else{
			if(ax > umbral){
				if(i!=7 && i!=15 && i!=23 && i!=31 && i!=39 && i!=47 && i!=55 && i!=63) i++;
				cooldown=STEP_COOLDOWN;
			}
			else if(ax < -umbral){
				if(i!=0 && i!=8 && i!=16 && i!=24 && i!=32 && i!=40 && i!=48 && i!=56) i--;
				cooldown=STEP_COOLDOWN;
			}

			if(ay > umbral){
				if(i+8<=63) i+=8;
				cooldown=STEP_COOLDOWN;
			}
			else if(ay < -umbral){
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
