#ifndef CONFIG_H
#define CONFIG_H

#define F_CPU 16000000UL

// UART
#define UART_BAUD 9600UL

// Timer1 tick
#define TICK_HZ 1000UL   // 1 kHz -> 1 ms

// Pines (PORTD)
#define PIN_SOL_DOWN  PD2
#define PIN_SOL_UP    PD3
#define PIN_MOVE_D    PD4
#define PIN_MOVE_U    PD5
#define PIN_MOVE_R    PD6
#define PIN_MOVE_L    PD7

#endif
