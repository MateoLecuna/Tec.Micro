#ifndef COMMANDS_H
#define COMMANDS_H
#include <stdint.h>

void commands_init(void);
void commands_process_byte(uint8_t b);
void commands_poll(void); // llamado desde main loop (no bloquea)

#endif
