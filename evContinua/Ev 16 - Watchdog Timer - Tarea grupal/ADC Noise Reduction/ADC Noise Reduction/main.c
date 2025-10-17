#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

// Configuración de tiempos
#define ACTIVE_MS   3000   // 1 s despierto
#define SLEEP_TICKS 1      // 1 tick de WDT

// LEDs en PD2..PD6
#define LED_DDR   DDRD
#define LED_PORT  PORTD
#define LED_MASK  ((1<<PD2)|(1<<PD3)|(1<<PD4)|(1<<PD5)|(1<<PD6))

volatile uint8_t wdt_ticks = 0;

// ISR WDT: cuenta “ticks” (aprox. 1 s por tick si WDP2|WDP1)
ISR(WDT_vect) {
	if (wdt_ticks < 255) wdt_ticks++;
}

// LEDs siempre encendidos
static inline void leds_setup_on(void){
	LED_DDR  |= LED_MASK;
	LED_PORT |= LED_MASK;
}

// WDT: interrupción aprox. 1 s (WDP2|WDP1)
static void wdt_enable_interrupt_1s(void){
	cli();
	MCUSR &= ~(1<<WDRF);                 // limpia flag WDT reset
	WDTCSR |= (1<<WDCE) | (1<<WDE);      // ventana de cambio
		// WDIE=1 (solo interrupción), WDE=0; prescaler aprox. 1 s => WDP2|WDP1
	WDTCSR = (1<<WDIE) | (1<<WDP2) | (1<<WDP1); // aporx. 1 s
	sei();
}

static void wdt_disable_all(void){
	cli();
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = 0x00;
	sei();
}

// ADC ON para este modo
static inline void adc_enable(void){  ADCSRA |=  (1<<ADEN); }
static inline void adc_disable(void){ ADCSRA &= ~(1<<ADEN); }

static void sleep_adc_nr_ticks(uint8_t ticks){
	wdt_ticks = 0;
	wdt_enable_interrupt_1s();

	adc_enable();
	set_sleep_mode(SLEEP_MODE_ADC);
	while (wdt_ticks < ticks) {
		sleep_enable();
		sei();
		sleep_cpu();   // despierta por WDT
		sleep_disable();
	}
	adc_disable();

	wdt_disable_all();
}

int main(void){
	leds_setup_on();

	while (1) {
		_delay_ms(ACTIVE_MS);            // CPU despierta, LEDs ON
		sleep_adc_nr_ticks(SLEEP_TICKS); // CPU sleep ADC-NR, LEDs ON
	}
}
