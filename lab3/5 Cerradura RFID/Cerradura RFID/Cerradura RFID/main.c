#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>

#include "SPI.h"
#include "UART.h"
#include "RC522.h"

/* ===================== UART ===================== */
#define UART_BAUD 9600UL
#define UBRR_VAL  ((F_CPU/16/UART_BAUD)-1)

/* ===================== LCD I2C (PCF8574) ===================== */
#define LCD_ADDR   0x27
#define LCD_SLAW   (LCD_ADDR<<1)
// PCF8574 bits: P0=RS, P1=RW, P2=E, P3=BL, P4..P7=D4..D7
#define RS_BM (1<<0)
#define RW_BM (1<<1)
#define E_BM  (1<<2)
#define BL_BM (1<<3)

static void twi_init(void){ TWSR=0x00; TWBR=72; TWCR=(1<<TWEN); }
static void twi_start(void){ TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN); while(!(TWCR&(1<<TWINT))); }
static void twi_stop(void){  TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO); }
static void twi_write(uint8_t d){ TWDR=d; TWCR=(1<<TWINT)|(1<<TWEN); while(!(TWCR&(1<<TWINT))); }
static void pcf_write(uint8_t v){ twi_start(); twi_write(LCD_SLAW); twi_write(v); twi_stop(); }

static void lcd_send_nibble(uint8_t nib, uint8_t rs){
	uint8_t out=((nib&0x0F)<<4)|rs|BL_BM;
	pcf_write(out); _delay_us(1);
	pcf_write(out|E_BM); _delay_us(1);
	pcf_write(out); _delay_us(40);
}
static void lcd_write_byte(uint8_t v, uint8_t rs){ lcd_send_nibble(v>>4,rs); lcd_send_nibble(v&0x0F,rs); }
static void lcd_cmd(uint8_t c){ lcd_write_byte(c,0); if(c==0x01||c==0x02)_delay_ms(2); else _delay_us(50); }
static void lcd_data(uint8_t d){ lcd_write_byte(d,RS_BM); _delay_us(50); }
static void lcd_set_cursor(uint8_t r,uint8_t c){ lcd_cmd(0x80 | ((r?0x40:0x00)+c)); }
static void lcd_print(const char*s){ while(*s) lcd_data((uint8_t)*s++); }
static void lcd_clear(void){ lcd_cmd(0x01); }
static void lcd_init(void){
	_delay_ms(20);
	lcd_send_nibble(0x03,0); _delay_ms(5);
	lcd_send_nibble(0x03,0); _delay_ms(1);
	lcd_send_nibble(0x03,0); _delay_us(150);
	lcd_send_nibble(0x02,0); _delay_us(150);
	lcd_cmd(0x28); lcd_cmd(0x08); lcd_cmd(0x01); lcd_cmd(0x06); lcd_cmd(0x0C);
}
static void lcd_print_at(uint8_t r,uint8_t c,const char* s){ lcd_set_cursor(r,c); lcd_print(s); }

/* ===================== Delays variables (fix _delay_ms) ===================== */
static void delay_ms_var(uint16_t ms){
	while(ms--) _delay_ms(1);
}

/* ===================== Parámetros ===================== */
#define MAX_UID_LEN      7
#define MAX_CARDS        64
#define SLOT_SIZE        (1 + MAX_UID_LEN)   // 1=len + uid
#define DISPLAY_UID_MS   2000                // ms que se mantiene el mensaje tras leer tarjeta
#define OPEN_TIMEOUT_MS  5000                // ms “abierto” (LED verde)
#define BTN_DEBOUNCE_MS  30
#define BTN_LONG_MS      1500

/* ===================== Pines ===================== */
// Botones (activos en 0 con pull-up)
#define BTN_PROG_PIN   PD2   // D2
#define BTN_ADD_PIN    PD3   // D3
#define BTN_DEL_PIN    PD6   // D6
// LEDs
#define LED_ROJO_PIN   PD4   // D4
#define LED_VERDE_PIN  PD5   // D5

static inline void pin_init(void){
	// Botones: entrada con pull-up
	DDRD &= ~((1<<BTN_PROG_PIN)|(1<<BTN_ADD_PIN)|(1<<BTN_DEL_PIN));
	PORTD |=  (1<<BTN_PROG_PIN)|(1<<BTN_ADD_PIN)|(1<<BTN_DEL_PIN);
	// LEDs: salida
	DDRD |= (1<<LED_ROJO_PIN)|(1<<LED_VERDE_PIN);
	PORTD &= ~((1<<LED_ROJO_PIN)|(1<<LED_VERDE_PIN));
}
static inline uint8_t btn_read(uint8_t bit){ return (PIND & (1<<bit))?1:0; } // 1=libre, 0=apretado
static uint8_t btn_was_pressed(uint8_t bit){
	if(!btn_read(bit)){
		_delay_ms(BTN_DEBOUNCE_MS);
		if(!btn_read(bit)){
			while(!btn_read(bit)){}
			_delay_ms(BTN_DEBOUNCE_MS);
			return 1;
		}
	}
	return 0;
}
static uint8_t btn_long_press(uint8_t bit){
	if(!btn_read(bit)){
		_delay_ms(BTN_DEBOUNCE_MS);
		if(!btn_read(bit)){
			uint16_t t=0;
			while(!btn_read(bit) && t < BTN_LONG_MS){ _delay_ms(10); t+=10; }
			while(!btn_read(bit)){}
			_delay_ms(BTN_DEBOUNCE_MS);
			return (t >= BTN_LONG_MS);
		}
	}
	return 0;
}
static inline void led_rojo(uint8_t on){ if(on) PORTD |= (1<<LED_ROJO_PIN); else PORTD &= ~(1<<LED_ROJO_PIN); }
static inline void led_verde(uint8_t on){ if(on) PORTD |= (1<<LED_VERDE_PIN); else PORTD &= ~(1<<LED_VERDE_PIN); }

/* ===================== Helpers UID ===================== */
static uint8_t uid_len(const uint8_t *u, uint8_t max){ uint8_t n=0; while(n<max && u[n]!=0) n++; return n; }
static bool uid_equal(const uint8_t *a, const uint8_t *b, uint8_t n){
	for(uint8_t i=0;i<n;i++) if(a[i]!=b[i]) return false; return true;
}

/* ===================== EEPROM: tabla de tarjetas ===================== */
static inline uint16_t slot_addr(uint8_t idx){ return (uint16_t)idx * (uint16_t)SLOT_SIZE; }

static void eeprom_read_slot(uint8_t idx, uint8_t *uid, uint8_t *len){
	uint16_t a = slot_addr(idx);
	uint8_t l = eeprom_read_byte((uint8_t*)a);
	if(l==0 || l>MAX_UID_LEN){ *len=0; return; }
	*len = l;
	for(uint8_t i=0;i<l;i++) uid[i] = eeprom_read_byte((uint8_t*)(a+1+i));
}
static void eeprom_write_slot(uint8_t idx, const uint8_t *uid, uint8_t len){
	if(len==0 || len>MAX_UID_LEN) return;
	uint16_t a = slot_addr(idx);
	eeprom_update_byte((uint8_t*)a, len);
	for(uint8_t i=0;i<len;i++) eeprom_update_byte((uint8_t*)(a+1+i), uid[i]);
}
static void eeprom_clear_slot(uint8_t idx){
	uint16_t a = slot_addr(idx);
	eeprom_update_byte((uint8_t*)a, 0); // len=0 => libre
}
static int8_t find_free_slot(void){
	for(uint8_t i=0;i<MAX_CARDS;i++){
		uint8_t l = eeprom_read_byte((uint8_t*)slot_addr(i));
		if(l==0) return i;
	}
	return -1;
}
static int8_t find_card_in_eeprom(const uint8_t *uid, uint8_t len){
	if(len==0 || len>MAX_UID_LEN) return -1;
	for(uint8_t i=0;i<MAX_CARDS;i++){
		uint8_t l; uint8_t tmp[MAX_UID_LEN]={0};
		eeprom_read_slot(i,tmp,&l);
		if(l==len && uid_equal(tmp,uid,len)) return i;
	}
	return -1;
}

/* ===================== UART comandos ===================== */
static void uart_list(void){
	uart_print("Listado de tarjetas:\r\n");
	for(uint8_t i=0;i<MAX_CARDS;i++){
		uint16_t a = slot_addr(i);
		uint8_t l = eeprom_read_byte((uint8_t*)a);
		if(l==0) continue;
		uart_print("#"); char buf[4]; sprintf(buf,"%u",i); uart_print(buf);
		uart_print("  len="); sprintf(buf,"%u",l); uart_print(buf);
		uart_print("  UID: ");
		for(uint8_t k=0;k<l;k++){ uint8_t b=eeprom_read_byte((uint8_t*)(a+1+k)); uart_print_hex(b); uart_send(' '); }
		uart_print("\r\n");
	}
}
static bool uart_expect_yes(void){
	uart_print("Confirme con YES\r\n");
	char r[4]={0};
	for(uint8_t i=0;i<3;i++){
		r[i]=uart_receive();
		if(r[i]=='\r' || r[i]=='\n'){ r[i]=0; break; }
	}
	return (r[0]=='Y' && r[1]=='E' && r[2]=='S');
}
static void uart_handle_line(char *line){
	if(strncmp(line,"LIST",4)==0){
		uart_list();
		}else if(strncmp(line,"CLEAR",5)==0){
		if(uart_expect_yes()){
			for(uint8_t i=0;i<MAX_CARDS;i++) eeprom_clear_slot(i);
			uart_print("EEPROM limpia\r\n");
			}else{
			uart_print("Cancelado\r\n");
		}
		}else if(strncmp(line,"DEL ",4)==0){
		int n = -1; sscanf(line+4,"%d",&n);
		if(n>=0 && n<MAX_CARDS){
			if(uart_expect_yes()){ eeprom_clear_slot((uint8_t)n); uart_print("Borrado\r\n"); }
			else uart_print("Cancelado\r\n");
			}else{
			uart_print("Indice invalido\r\n");
		}
		}else{
		uart_print("Comandos: LIST | CLEAR | DEL n\r\n");
	}
}

/* ===================== UI helpers ===================== */
static void ui_idle(void){
	lcd_clear();
	lcd_print_at(0,0,"Acerque tarjeta ");
	lcd_print_at(1,0,"Prog: mant D2   ");
	led_rojo(1); led_verde(0);
}
static void ui_prog_menu(void){
	lcd_clear();
	lcd_print_at(0,0,"MODO PROG");
	lcd_print_at(1,0,"ADD=D3  DEL=D6  ");
	led_rojo(0); led_verde(0);
}
static void ui_msg(const char *l1, const char *l2, uint16_t ms){
	lcd_clear(); lcd_print_at(0,0,l1); lcd_print_at(1,0,l2);
	if(ms){ delay_ms_var(ms); }
}

/* ===================== MAIN ===================== */
int main(void){
	uart_init(UBRR_VAL);
	spi_init();
	twi_init();
	lcd_init();
	pin_init();

	PORTB |= (1<<PB2);       // SDA alto

	// RC522
	mfrc522_resetPinInit();
	mfrc522_debug_init();

	// UI inicial
	ui_idle();
	uart_print("\r\nCandado RFID listo. Comandos UART: LIST, CLEAR, DEL n\r\n");

	enum { ST_IDLE=0, ST_OPEN, ST_PROG, ST_WAIT_ADD, ST_WAIT_DEL } state = ST_IDLE;

	// Buffer UART simple
	char line[32]; uint8_t li=0;

	uint8_t last_uid[10]={0}; uint8_t last_n=0;

	while(1){
		/* ---------- UART (línea por línea) ---------- */
		if(UCSR0A & (1<<RXC0)){
			char c = uart_receive();
			if(c=='\r' || c=='\n'){
				line[li]=0; if(li>0){ uart_handle_line(line); li=0; }
				}else if(li < sizeof(line)-1){
				line[li++]=c;
			}
		}

		/* ---------- Botones ---------- */
		// Pulsación larga en PROG ? entrar/salir modo programación
		if(state==ST_IDLE || state==ST_OPEN){
			if(btn_long_press(BTN_PROG_PIN)){
				state = ST_PROG;
				ui_prog_menu();
				continue;
			}
			}else if(state==ST_PROG || state==ST_WAIT_ADD || state==ST_WAIT_DEL){
			if(btn_long_press(BTN_PROG_PIN)){
				state = ST_IDLE;
				ui_idle();
				last_n=0; memset(last_uid,0,sizeof(last_uid));
				continue;
			}
		}

		if(state==ST_PROG){
			if(btn_was_pressed(BTN_ADD_PIN)){
				state = ST_WAIT_ADD;
				ui_msg("Pase tarjeta", "para ALTA", 0);
				continue;
			}
			if(btn_was_pressed(BTN_DEL_PIN)){
				state = ST_WAIT_DEL;
				ui_msg("Pase tarjeta", "para BAJA", 0);
				continue;
			}
		}

		/* ---------- Lectura de tarjeta ---------- */
		uint8_t uid[10]={0};
		mfrc522_standard(uid);
		uint8_t n = uid_len(uid,10);

		if(n==0){
			_delay_ms(100);
			continue;
		}

		// Evitar spam con la misma tarjeta pegada
		bool same = (n==last_n) && uid_equal(uid,last_uid,n);

		/* ---------- Lógica por estado ---------- */
		if(state==ST_IDLE){
			if(!same){
				int8_t idx = find_card_in_eeprom(uid,n);
				if(idx>=0){
					ui_msg("Acceso permitido","",0);
					led_rojo(0); led_verde(1);
					// Aquí podrías activar un relé real si lo agregás
					delay_ms_var(OPEN_TIMEOUT_MS);
					led_verde(0); led_rojo(1);
					ui_idle();
					state = ST_IDLE;
					}else{
					ui_msg("Acceso denegado","",1000);
					ui_idle();
				}
				memcpy(last_uid,uid,n); last_n=n;
				delay_ms_var(DISPLAY_UID_MS);
			}
		}
		else if(state==ST_PROG){
			_delay_ms(100); // esperando elección ADD/DEL
		}
		else if(state==ST_WAIT_ADD){
			if(!same){
				int8_t idx = find_card_in_eeprom(uid,n);
				if(idx>=0){
					ui_msg("Ya existe", "No se duplica", 1200);
					}else{
					int8_t free = find_free_slot();
					if(free<0){
						ui_msg("Memoria llena","",1200);
						}else{
						eeprom_write_slot((uint8_t)free, uid, n);
						ui_msg("Guardada OK","",900);
					}
				}
				memcpy(last_uid,uid,n); last_n=n;
				state = ST_PROG;
				ui_prog_menu();
			}
		}
		else if(state==ST_WAIT_DEL){
			if(!same){
				int8_t idx = find_card_in_eeprom(uid,n);
				if(idx<0){
					ui_msg("No encontrada","",1200);
					}else{
					eeprom_clear_slot((uint8_t)idx);
					ui_msg("Borrada OK","",900);
				}
				memcpy(last_uid,uid,n); last_n=n;
				state = ST_PROG;
				ui_prog_menu();
			}
		}
		else if(state==ST_OPEN){
			// No usamos estado separado, se maneja en línea arriba
		}

		_delay_ms(50);
	}
}
