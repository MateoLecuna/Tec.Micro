#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

void uart_init(uint32_t baud);
bool uart_tx_enqueue(uint8_t b);
bool uart_rx_dequeue(uint8_t* b);
void uart_print(const char* s);

#endif
