#include "commands.h"
#include "uart.h"
#include "scheduler.h"
#include "gpio.h"

// Pequeñas rutinas de movimiento (placeholder de figuras)
static void do_nudge_up(void){   pen_up();   sched_start(MV_UP,    250); }
static void do_nudge_down(void){ pen_up();   sched_start(MV_DOWN,  250); }
static void do_nudge_left(void){ pen_up();   sched_start(MV_LEFT,  250); }
static void do_nudge_right(void){pen_up();   sched_start(MV_RIGHT, 250); }

// TODO: cuando tengamos draw_line/circle, acá llamamos a las figuras
static void do_triangle_demo(void){
	pen_down();
	// placeholder: tres lados simbólicos
	sched_start(MV_RIGHT, 600); while(sched_busy()){} // bloqueito breve: luego lo haremos por colas
	sched_start(MV_DIAG_UL, 600); while(sched_busy()){}
	sched_start(MV_DOWN, 600); while(sched_busy()){}
	pen_up();
}
static void do_circle_demo(void){
	pen_down();
	// placeholder: “cuadrado redondo” en 4 diagonales
	sched_start(MV_DIAG_UR, 600); while(sched_busy()){}
	sched_start(MV_DIAG_DR, 600); while(sched_busy()){}
	sched_start(MV_DIAG_DL, 600); while(sched_busy()){}
	sched_start(MV_DIAG_UL, 600); while(sched_busy()){}
	pen_up();
}
static void do_cross_demo(void){
	pen_down();
	sched_start(MV_UP, 700); while(sched_busy()){}
	sched_start(MV_DOWN, 1400); while(sched_busy()){}
	sched_start(MV_UP, 700); while(sched_busy()){}
	sched_start(MV_LEFT, 700); while(sched_busy()){}
	sched_start(MV_RIGHT, 1400); while(sched_busy()){}
	sched_start(MV_LEFT, 700); while(sched_busy()){}
	pen_up();
}

void commands_init(void){
	uart_print("\r\n=== Plotter Ready ===\r\n");
	uart_print("[0] Arriba  [1] Abajo  [2] Izq  [3] Der\r\n");
	uart_print("[4] Triangulo  [5] Circulo  [6] Cruz  [T] Todo\r\n\r\n");
}

void commands_process_byte(uint8_t b){
	switch(b){
		case '0': do_nudge_up();    break;
		case '1': do_nudge_down();  break;
		case '2': do_nudge_left();  break;
		case '3': do_nudge_right(); break;
		case '4': do_triangle_demo(); break;   // luego: draw_triangle()
		case '5': do_circle_demo();   break;   // luego: draw_circle()
		case '6': do_cross_demo();    break;   // luego: draw_cross()
		case 'T':
		do_triangle_demo();
		do_nudge_right(); do_nudge_right();
		do_circle_demo();
		do_nudge_right(); do_nudge_right();
		do_cross_demo();
		break;
		default:
		uart_print("Opcion invalida!\r\n");
		commands_init();
		break;
	}
}

void commands_poll(void){
	uint8_t b;
	// Despachador no bloqueante: lee lo que haya en RX y procesa
	while (uart_rx_dequeue(&b)){
		commands_process_byte(b);
	}
}
