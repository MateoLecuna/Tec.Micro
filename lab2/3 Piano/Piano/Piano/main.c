#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "hw_pins.h"
#include "sonidos.h"
#include "figuras.h"
#include "canciones.h"
#include "funciones.h"
#include "uart.h"
#include "piano.h"

int main(void) {
	// ==== Inicializaciones ====
	hw_init_pins();           // pines de salida/audio, LEDs, etc.
	sound_init_timer2();      // tu Timer2 configurado para generar sonido
	piano_init();             // inicializa las 12 teclas
	uart_init(9600);          // UART a 9600 baud
	_delay_ms(50);

	uart_write("Bienvenido al Piano via UART!\n");
	uart_write("Comandos disponibles:\n");
	uart_write("  C1   -> Reproducir Cancion 1\n");
	uart_write("  C2   -> Reproducir Cancion 2\n");
	uart_write("  PIANO-> Modo piano manual\n> ");

	enum { MODE_PIANO = 0, MODE_SONG = 1 } mode = MODE_PIANO;
	uint8_t current_song = 0;
	char line[32];

	for (;;) {
		// Lectura del UART (por línea, termina con \n o \r)
		int8_t n = uart_readline(line, sizeof(line));
		if (n > 0) {
			// Pasar a mayúsculas
			for (int i = 0; i < n; ++i)
			if (line[i] >= 'a' && line[i] <= 'z') line[i] -= 32;

			if (!strcmp(line, "C1")) {
				uart_write(">> Reproduciendo CANCION 1\n> ");
				current_song = 1;
				mode = MODE_SONG;
				} else if (!strcmp(line, "C2")) {
				uart_write(">> Reproduciendo CANCION 2\n> ");
				current_song = 2;
				mode = MODE_SONG;
				} else if (!strcmp(line, "PIANO")) {
				uart_write(">> Modo PIANO activado\n> ");
				mode = MODE_PIANO;
				} else {
				uart_write("?? Comando invalido\n> ");
			}
		}

		// Lógica principal
		if (mode == MODE_SONG) {
			// Canciones bloqueantes
			if (current_song == 1) {
				c_asesina();   // canción 1
				} else if (current_song == 2) {
				c_SuperMario();  // canción 2
			} 

			// Cuando termina la canción, vuelve al modo piano
			current_song = 0;
			mode = MODE_PIANO;
			uart_write(">> Cancion finalizada. Modo PIANO.\n> ");
		}
		else {
			// Modo Piano manual
			piano_mode_tick();   // lee teclas y toca la nota correspondiente
		}

		_delay_ms(1);
	}
}
