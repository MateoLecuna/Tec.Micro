#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>

void timer1_init_ctc_1khz(void);

// Hook que se llama desde ISR cada 1 ms (lo define app)
void tick_1ms_isr_hook(void);

#endif
