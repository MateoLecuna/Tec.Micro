#include "logica_candado.h"
#include "estados.h"
#include "hardware.h"
#include "teclado.h"
#include "memoria.h"
#include "lcd_i2c.h"

#define F_CPU 16000000UL

#include <string.h>
#include <util/delay.h>

static Estado estado = BLOQUEADO;

static char entrada[CLAVE_MAX_LONG + 1];
static uint8_t largoEntrada = 0;

static char confirmacion[CLAVE_MAX_LONG + 1];
static uint8_t largoConfirmacion = 0;

static uint8_t intentosErroneos = 0;
static const uint8_t MAX_INTENTOS = 5;

static void limpiar_entrada(void) { largoEntrada=0; entrada[0]='\0'; }
static void limpiar_confirmacion(void){ largoConfirmacion=0; confirmacion[0]='\0'; }

static void mostrar_entrada_enmascarada(void) {
	uint8_t i;
	lcd_limpiar_fila(1);
	for (i=0; i<largoEntrada && i<16; i++) lcd_print_char('*');
}

static uint8_t entrada_igual_a_clave(void) {
	if (largoEntrada != largoClave) return 0;
	for (uint8_t i=0; i<largoClave; i++) if (entrada[i] != claveActiva[i]) return 0;
	return 1;
}

static void agregar_digito(char k) {
	if (k>='0' && k<='9' && largoEntrada < CLAVE_MAX_LONG) {
		entrada[largoEntrada++] = k;
		entrada[largoEntrada] = '\0';
	}
}

// Transiciones
static void ir_bloqueado(void) {
	estado = BLOQUEADO;
	leds(0,1);
	limpiar_entrada();
	lcd_titulo("Candado: BLOQ","Ingrese clave");
}

static void ir_desbloqueado(const char* msg) {
	estado = DESBLOQUEADO;
	leds(1,0);
	limpiar_entrada();
	lcd_titulo(msg, "Teclee p/ cerrar");
}

static void ir_bloqueo(void) {
	estado = BLOQUEO;
	leds(0,1);
	limpiar_entrada();
	lcd_titulo("BLOQUEADO", "Use D (llave)");
}

static void procesar_envio(void) {
	if (entrada_igual_a_clave()) {
		intentosErroneos = 0;
		ir_desbloqueado("Candado: ABIERTO");
		} else {
		intentosErroneos++;
		leds(0,1);
		lcd_titulo("Clave INCORRECTA","Intente de nuevo");
		_delay_ms(900);
		if (intentosErroneos >= MAX_INTENTOS) {
			ir_bloqueo();
			} else {
			estado = INGRESANDO;
			lcd_titulo("Ingrese clave:", "");
		}
		limpiar_entrada();
	}
}

static void iniciar_cambio_clave(void) {
	estado = CAMBIO_ANTIGUA;
	limpiar_entrada();
	lcd_titulo("Cambio de clave","Clave actual:");
}

static void aceptar_clave_antigua(void) {
	if (entrada_igual_a_clave()) {
		estado = CAMBIO_NUEVA;
		limpiar_entrada();
		lcd_titulo("Nueva clave:","(A=guardar, C=bor)");
		} else {
		lcd_titulo("Clave actual mal","Operacion cancelada");
		_delay_ms(900);
		ir_bloqueado();
	}
}

static void aceptar_nueva_clave_primera(void) {
	if (largoEntrada == 0) {
		lcd_titulo("Nueva vacia","Intente otra vez");
		_delay_ms(800);
		lcd_titulo("Nueva clave:","(A=guardar, C=bor)");
		return;
	}
	memcpy(confirmacion, entrada, largoEntrada+1);
	largoConfirmacion = largoEntrada;

	estado = CAMBIO_CONFIRMAR;
	limpiar_entrada();
	lcd_titulo("Confirmar clave:","(A=confirmar)");
}

static uint8_t confirmar_y_guardar_nueva_clave(void) {
	if (largoEntrada != largoConfirmacion) return 0;
	for (uint8_t i=0; i<largoEntrada; i++) if (entrada[i] != confirmacion[i]) return 0;
	memoria_guardar_clave(entrada, largoEntrada);
	return 1;
}

/* ---- API ---- */
void candado_iniciar(void) {
	teclado_iniciar();
	ir_bloqueado();
}

void candado_loop(void) {
	char k = teclado_leer();
	if (k == '\0') return;

	// “D” = llave universal
	if (k == 'D') {
		intentosErroneos = 0;
		ir_desbloqueado("Llave usada (D)");
		return;
	}

	switch (estado) {
		case BLOQUEADO:
		estado = INGRESANDO;
		lcd_titulo("Ingrese clave:","");
		// intencionalmente sin break

		case INGRESANDO:
		if (k>='0' && k<='9') { agregar_digito(k); mostrar_entrada_enmascarada(); }
		else if (k=='C')      { limpiar_entrada(); mostrar_entrada_enmascarada(); }
		else if (k=='A')      { procesar_envio(); }
		else if (k=='B')      { iniciar_cambio_clave(); }
		break;

		case DESBLOQUEADO:
		ir_bloqueado(); // cualquier tecla cierra
		break;

		case CAMBIO_ANTIGUA:
		if (k>='0' && k<='9') { agregar_digito(k); mostrar_entrada_enmascarada(); }
		else if (k=='C')      { limpiar_entrada(); mostrar_entrada_enmascarada(); }
		else if (k=='A')      { aceptar_clave_antigua(); }
		else if (k=='B')      { ir_bloqueado(); }
		break;

		case CAMBIO_NUEVA:
		if (k>='0' && k<='9') { agregar_digito(k); mostrar_entrada_enmascarada(); }
		else if (k=='C')      { limpiar_entrada(); mostrar_entrada_enmascarada(); }
		else if (k=='A')      { aceptar_nueva_clave_primera(); }
		else if (k=='B')      { ir_bloqueado(); }
		break;

		case CAMBIO_CONFIRMAR:
		if (k>='0' && k<='9') { agregar_digito(k); mostrar_entrada_enmascarada(); }
		else if (k=='C')      { limpiar_entrada(); lcd_limpiar_fila(1); }
		else if (k=='A') {
			if (confirmar_y_guardar_nueva_clave()) { lcd_titulo("Clave cambiada!",""); _delay_ms(900); }
			else { lcd_titulo("No coincide","Cambio cancelado"); _delay_ms(900); }
			limpiar_entrada(); limpiar_confirmacion();
			ir_bloqueado();
			} else if (k=='B') {
			limpiar_entrada(); limpiar_confirmacion();
			ir_bloqueado();
		}
		break;

		case BLOQUEO:
		if (k=='C') lcd_titulo("BLOQUEADO","Use D (llave)");
		break;
	}
}
