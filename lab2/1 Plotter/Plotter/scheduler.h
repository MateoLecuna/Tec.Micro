#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdint.h>
#include <stdbool.h>

// Tipos de movimiento de tramo
typedef enum {
	MV_NONE=0, MV_UP, MV_DOWN, MV_LEFT, MV_RIGHT,
	MV_DIAG_UR, MV_DIAG_UL, MV_DIAG_DR, MV_DIAG_DL
} move_t;

void sched_init(void);
void sched_start(move_t mv, uint16_t duration_ms);
bool sched_busy(void);
void sched_stop(void);

// llamado por ISR cada 1 ms
void sched_tick_isr(void);

#endif
