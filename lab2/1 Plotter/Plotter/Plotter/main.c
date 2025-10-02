#include <avr/io.h>
#include <avr/interrupt.h>
#include "config.h"
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include "scheduler.h"
#include "commands.h"
#include "draw.h"

void tick_1ms_isr_hook(void){
	sched_tick_isr();
}

int main(void){
	gpio_init();
	uart_init(UART_BAUD);
	timer1_init_ctc_1khz();
	sched_init();
	draw_init();

	sei();

	commands_init();

	for(;;){
		// Poll de comandos por UART (no bloquea)
		commands_poll();

		// Aquí más adelante:
		// - gestor de colas de dibujo (line/circle no bloqueantes)
		// - timeouts, estados, etc.
	}
}
