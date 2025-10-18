#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/eeprom.h>

// Parámetros I2C/LCD
#define F_SCL      100000UL
#define LCD_ADDR   0x27
#define LCD_SLAW   (LCD_ADDR<<1)

// Backpack PCF8574: P0=RS, P1=RW, P2=E, P3=BL, P4..P7=D4..D7
#define RS_BM (1<<0)
#define RW_BM (1<<1)
#define E_BM  (1<<2)
#define BL_BM (1<<3)

// Pines de feedback
#define GREEN_LED_PIN  PB5   // D13
#define RED_LED_PIN    PB4   // D12
#define BUZZER_PIN     PD3   // D3

// I2C (TWI)
static void twi_init(void){ TWSR=0x00; TWBR=72; TWCR=(1<<TWEN); }
static void twi_start(void){ TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN); while(!(TWCR&(1<<TWINT))); }
static void twi_stop(void){  TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO); }
static void twi_write(uint8_t d){ TWDR=d; TWCR=(1<<TWINT)|(1<<TWEN); while(!(TWCR&(1<<TWINT))); }
static void pcf_write(uint8_t v){ twi_start(); twi_write(LCD_SLAW); twi_write(v); twi_stop(); }

// LCD
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
static void lcd_print_num2(uint8_t r,uint8_t c,uint8_t v){
    char b[3]; b[0]='0'+(v/10)%10; b[1]='0'+(v%10); b[2]='\0'; lcd_print_at(r,c,b);
}

// Teclado 4x4
// Filas -> PD7,PD6,PD5,PD4 (salidas activas en 0)
// Columnas -> PB3,PB2,PB1,PB0 (entradas con pull-up)

static void kpd_init(void){
    DDRD|=0xF0; PORTD|=0xF0;      // filas
    DDRB&=~0x0F; PORTB|=0x0F;     // columnas
}
static char keymap(uint8_t r,uint8_t c){
    static const char tbl[4][4]={
		{'1','2','3','A'},
		{'4','5','6','B'},
		{'7','8','9','C'},
		{'*','0','#','D'}};
    return (r<4&&c<4)?tbl[r][c]:'\0';
}
static char kpd_getkey_blocking(void){
    const uint8_t rowBit[4]={7,6,5,4};   // r -> PD7..PD4
    const uint8_t colBit[4]={0,1,2,3};   // c -> PB0..PB3
    while(1){
        for(uint8_t r=0;r<4;r++){
            PORTD|=0xF0; PORTD&=~(1<<rowBit[r]); _delay_us(5);
            uint8_t pinb=PINB&0x0F;
            if(pinb!=0x0F){
                _delay_ms(20);
                pinb=PINB&0x0F;
                if(pinb!=0x0F){
                    for(uint8_t c=0;c<4;c++){
                        if((pinb&(1<<colBit[c]))==0){
                            char ch=keymap(r,c);
                            while((PINB&(1<<colBit[c]))==0) {;}
                            _delay_ms(10);
                            return ch;
                        }
                    }
                }
            }
        }
    }
}
// Versión no bloqueante para la alarma
static char kpd_getkey_nonblocking(void){
    const uint8_t rowBit[4]={7,6,5,4};
    const uint8_t colBit[4]={0,1,2,3};
    for(uint8_t r=0;r<4;r++){
        PORTD|=0xF0; PORTD&=~(1<<rowBit[r]); _delay_us(5);
        uint8_t pinb=PINB&0x0F;
        if(pinb!=0x0F){
            _delay_ms(10);
            pinb=PINB&0x0F;
            if(pinb!=0x0F){
                for(uint8_t c=0;c<4;c++){
                    if((pinb&(1<<colBit[c]))==0){
                        char ch=keymap(r,c);
                        while((PINB&(1<<colBit[c]))==0) {;}
                        _delay_ms(5);
                        return ch;
                    }
                }
            }
        }
    }
    return '\0';
}

// EEPROM PIN
#define EEPROM_LEN_ADDR  ((uint8_t*)0)
#define EEPROM_DIG_ADDR  ((uint8_t*)1)
#define MAX_PIN_LEN      6
static void eeprom_load_pin(char* buf, uint8_t* len){
    uint8_t l=eeprom_read_byte(EEPROM_LEN_ADDR);
    if(l<4 || l>MAX_PIN_LEN){
        const char def[]="1234";
        eeprom_update_byte(EEPROM_LEN_ADDR,4);
        for(uint8_t i=0;i<4;i++) eeprom_update_byte(EEPROM_DIG_ADDR+i,(uint8_t)def[i]);
        for(uint8_t i=4;i<MAX_PIN_LEN;i++) eeprom_update_byte(EEPROM_DIG_ADDR+i,0);
        l=4;
    }
    for(uint8_t i=0;i<l;i++) buf[i]=(char)eeprom_read_byte(EEPROM_DIG_ADDR+i);
    buf[l]='\0'; *len=l;
}
static void eeprom_save_pin(const char* buf, uint8_t len){
    if(len<4 || len>MAX_PIN_LEN) return;
    eeprom_update_byte(EEPROM_LEN_ADDR,len);
    for(uint8_t i=0;i<len;i++) eeprom_update_byte(EEPROM_DIG_ADDR+i,(uint8_t)buf[i]);
    for(uint8_t i=len;i<MAX_PIN_LEN;i++) eeprom_update_byte(EEPROM_DIG_ADDR+i,0);
}

// UI helpers
static void ui_reset_pin_line(void){ lcd_set_cursor(0,5); for(uint8_t i=0;i<11;i++) lcd_data(' '); lcd_set_cursor(0,5); }
static void ui_show_attempts(uint8_t at){
    lcd_set_cursor(1,0); lcd_print("Intentos: ");
    lcd_print_num2(1,10,at); lcd_set_cursor(1,12); lcd_data('/'); lcd_set_cursor(1,13); lcd_data('3');
}

// Estados de cambio
typedef enum { MODE_NORMAL=0, MODE_CHK_CURPIN, MODE_CHG_NEW1, MODE_CHG_NEW2 } change_mode_t;

// Feedback: LEDs y Buzzer
static inline void green_on(void){ PORTB |= (1<<GREEN_LED_PIN); }
static inline void green_off(void){ PORTB &= ~(1<<GREEN_LED_PIN); }
static inline void red_on(void){   PORTB |= (1<<RED_LED_PIN); }
static inline void red_off(void){  PORTB &= ~(1<<RED_LED_PIN); }
static inline void buz_on(void){   PORTD |= (1<<BUZZER_PIN); }
static inline void buz_off(void){  PORTD &= ~(1<<BUZZER_PIN); }

// Delays variables seguros (evitar _delay_ms(var))
static void delay_ms_var(uint16_t ms){ while(ms--) _delay_ms(1); }

// Beeps
static void beep_ms(uint16_t on_ms){ buz_on(); delay_ms_var(on_ms); buz_off(); }
static void beep_ok_triple(void){
    for(uint8_t i=0;i<3;i++){ buz_on(); _delay_ms(90); buz_off(); _delay_ms(90); }
}

// Alarma hasta 'D'
static void alarm_until_master(void){
    lcd_print_at(1,0,"Bloqueado     ");
    while(1){
        // ON
        red_on(); buz_on();
        for(uint16_t t=0;t<200;t+=20){ _delay_ms(20); char k=kpd_getkey_nonblocking(); if(k=='D'){ buz_off(); red_off(); return; } }
        // OFF
        buz_off(); red_off();
        for(uint16_t t=0;t<200;t+=20){ _delay_ms(20); char k=kpd_getkey_nonblocking(); if(k=='D'){ return; } }
    }
}

// --- MAIN ---
#define MAX_ATTEMPTS 3

int main(void){
    twi_init(); lcd_init(); kpd_init();

    // Configurar pines de feedback
    DDRB |= (1<<GREEN_LED_PIN) | (1<<RED_LED_PIN);
    PORTB &= ~((1<<GREEN_LED_PIN) | (1<<RED_LED_PIN));
    DDRD |= (1<<BUZZER_PIN);
    buz_off();

    char pin_stored[MAX_PIN_LEN+1]; uint8_t pin_len_stored=0;
    eeprom_load_pin(pin_stored,&pin_len_stored);

    lcd_clear(); lcd_print_at(0,0,"Pin: "); ui_reset_pin_line(); ui_show_attempts(0);

    char entry[7]={0};     uint8_t entry_len=0;
    char curpin[7]={0};    uint8_t curpin_len=0;
    char newpin_first[7]={0}; uint8_t newpin_first_len=0;
    char newpin_conf[7]={0};  uint8_t newpin_conf_len=0;

    uint8_t attempts=0; bool locked=false; change_mode_t mode=MODE_NORMAL;

    while(1){
        char k=kpd_getkey_blocking();

        // Llave maestra 'D'
        if(k=='D'){
            attempts=0; locked=false; mode=MODE_NORMAL;
            buz_off(); red_off();
            green_on(); beep_ok_triple(); green_off();
            lcd_print_at(1,0,"MASTER OK    "); ui_show_attempts(attempts);
            _delay_ms(700);
            lcd_print_at(1,0,"             "); ui_show_attempts(attempts);
            ui_reset_pin_line(); entry_len=0; entry[0]=0;
            continue;
        }

        if(locked){
            // Si está bloqueado, sonar alarma hasta D
            alarm_until_master();
            // al volver, reseteamos
            attempts=0; locked=false;
            lcd_print_at(1,0,"Listo         "); ui_show_attempts(attempts);
            _delay_ms(600);
            lcd_print_at(1,0,"              "); ui_show_attempts(attempts);
            ui_reset_pin_line(); entry_len=0; entry[0]=0;
            continue;
        }

        // Iniciar cambio con 'C'
        if(mode==MODE_NORMAL && k=='C'){
            mode=MODE_CHK_CURPIN; curpin_len=0; curpin[0]=0;
            lcd_print_at(1,0,"Ingrese actual");
            ui_reset_pin_line();
            continue;
        }

        // 'B' borra el buffer del modo actual
        if(k=='B'){
            if(mode==MODE_NORMAL){ entry_len=0; entry[0]=0; }
            else if(mode==MODE_CHK_CURPIN){ curpin_len=0; curpin[0]=0; }
            else if(mode==MODE_CHG_NEW1){ newpin_first_len=0; newpin_first[0]=0; }
            else if(mode==MODE_CHG_NEW2){ newpin_conf_len=0; newpin_conf[0]=0; }
            ui_reset_pin_line(); continue;
        }

        // '*' backspace
        if(k=='*'){
            if(mode==MODE_NORMAL && entry_len){ entry_len--; entry[entry_len]=0; lcd_set_cursor(0,5+entry_len); lcd_data(' '); lcd_set_cursor(0,5+entry_len); }
            else if(mode==MODE_CHK_CURPIN && curpin_len){ curpin_len--; curpin[curpin_len]=0; lcd_set_cursor(0,5+curpin_len); lcd_data(' '); lcd_set_cursor(0,5+curpin_len); }
            else if(mode==MODE_CHG_NEW1 && newpin_first_len){ newpin_first_len--; newpin_first[newpin_first_len]=0; lcd_set_cursor(0,5+newpin_first_len); lcd_data(' '); lcd_set_cursor(0,5+newpin_first_len); }
            else if(mode==MODE_CHG_NEW2 && newpin_conf_len){ newpin_conf_len--; newpin_conf[newpin_conf_len]=0; lcd_set_cursor(0,5+newpin_conf_len); lcd_data(' '); lcd_set_cursor(0,5+newpin_conf_len); }
            continue;
        }

        // Dígitos
        if(k>='0' && k<='9'){
            if(mode==MODE_NORMAL){
                if(entry_len<MAX_PIN_LEN){ entry[entry_len++]=k; entry[entry_len]=0; lcd_set_cursor(0,5+entry_len-1); lcd_data(k); }
            }else if(mode==MODE_CHK_CURPIN){
                if(curpin_len<MAX_PIN_LEN){ curpin[curpin_len++]=k; curpin[curpin_len]=0; lcd_set_cursor(0,5+curpin_len-1); lcd_data(k); }
            }else if(mode==MODE_CHG_NEW1){
                if(newpin_first_len<MAX_PIN_LEN){ newpin_first[newpin_first_len++]=k; newpin_first[newpin_first_len]=0; lcd_set_cursor(0,5+newpin_first_len-1); lcd_data(k); }
            }else if(mode==MODE_CHG_NEW2){
                if(newpin_conf_len<MAX_PIN_LEN){ newpin_conf[newpin_conf_len++]=k; newpin_conf[newpin_conf_len]=0; lcd_set_cursor(0,5+newpin_conf_len-1); lcd_data(k); }
            }
            continue;
        }

        // 'A' enviar según modo
        if(k=='A'){
            if(mode==MODE_NORMAL){
                if(entry_len<4){
                    // Feedback para "falta longitud"
                    red_on(); beep_ms(150); red_off();

                    lcd_print_at(1,0,"Min 4 digitos "); _delay_ms(600);
                    lcd_print_at(1,0,"              "); ui_show_attempts(attempts);
                    ui_reset_pin_line(); entry_len=0; entry[0]=0;
                }else{
                    char pin_stored[MAX_PIN_LEN+1]; uint8_t pin_len_stored=0;
                    eeprom_load_pin(pin_stored,&pin_len_stored);
                    bool ok=(entry_len==pin_len_stored);
                    for(uint8_t i=0; ok && i<entry_len; i++) if(entry[i]!=pin_stored[i]) ok=false;

                    if(ok){
                        attempts=0; ui_show_attempts(attempts);
                        // Feedback OK: LED verde + 3 beeps
                        green_on(); beep_ok_triple(); green_off();
                        lcd_print_at(1,0,"OK            "); _delay_ms(800);
                        lcd_print_at(1,0,"              "); ui_show_attempts(attempts);
                        ui_reset_pin_line(); entry_len=0; entry[0]=0;
                    }else{
                        attempts++; ui_show_attempts(attempts);
                        // Feedback ERROR: LED rojo + beep 500ms
                        red_on(); beep_ms(500); red_off();
                        lcd_print_at(1,0,"Error         "); _delay_ms(700);
                        lcd_print_at(1,0,"              "); ui_show_attempts(attempts);
                        ui_reset_pin_line(); entry_len=0; entry[0]=0;

                        // Alarmar INMEDIATAMENTE al llegar al máximo
                        if (attempts >= MAX_ATTEMPTS) {
                            locked = true;
                            alarm_until_master();   // entra en sirena hasta 'D'

                            // al salir con 'D', restaurar UI/estado
                            attempts = 0;
                            locked = false;
                            ui_show_attempts(attempts);
                            lcd_print_at(1,0,"Listo         ");
                            _delay_ms(600);
                            lcd_print_at(1,0,"              ");
                            ui_reset_pin_line();
                            entry_len = 0; entry[0] = 0;
                            continue;
                        }
                    }
                }
            }else if(mode==MODE_CHK_CURPIN){
                char pin_stored[MAX_PIN_LEN+1]; uint8_t pin_len_stored=0;
                eeprom_load_pin(pin_stored,&pin_len_stored);
                bool ok=(curpin_len==pin_len_stored);
                for(uint8_t i=0; ok && i<curpin_len; i++) if(curpin[i]!=pin_stored[i]) ok=false;
                if(!ok){
                    red_on(); beep_ms(500); red_off();
                    lcd_print_at(1,0,"Actual incorrecto"); _delay_ms(900);
                    lcd_print_at(1,0,"                 "); mode=MODE_NORMAL; ui_reset_pin_line();
                    curpin_len=0; curpin[0]=0;
                }else{
                    mode=MODE_CHG_NEW1; newpin_first_len=0; newpin_first[0]=0;
                    lcd_print_at(1,0,"Nuevo PIN (4-6) "); ui_reset_pin_line();
                }
            }else if(mode==MODE_CHG_NEW1){
                if(newpin_first_len<4 || newpin_first_len>MAX_PIN_LEN){
                    red_on(); beep_ms(300); red_off();
                    lcd_print_at(1,0,"Long 4-6 digitos"); _delay_ms(800);
                    lcd_print_at(1,0,"                "); mode=MODE_NORMAL; ui_reset_pin_line();
                }else{
                    mode=MODE_CHG_NEW2; newpin_conf_len=0; newpin_conf[0]=0;
                    lcd_print_at(1,0,"Confirmar nuevo "); ui_reset_pin_line();
                }
            }else if(mode==MODE_CHG_NEW2){
                bool match=(newpin_conf_len==newpin_first_len);
                for(uint8_t i=0; match && i<newpin_first_len; i++) if(newpin_conf[i]!=newpin_first[i]) match=false;
                if(!match){
                    red_on(); beep_ms(300); red_off();
                    lcd_print_at(1,0,"No coincide     "); _delay_ms(900);
                    lcd_print_at(1,0,"                ");
                }else{
                    eeprom_save_pin(newpin_first,newpin_first_len);
                    green_on(); beep_ok_triple(); green_off();
                    lcd_print_at(1,0,"PIN actualizado "); _delay_ms(900);
                    lcd_print_at(1,0,"                ");
                }
                mode=MODE_NORMAL; ui_reset_pin_line(); entry_len=0; entry[0]=0;
            }
            continue;
        }
    }
}
