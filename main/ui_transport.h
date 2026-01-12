// ui_transport.h
#pragma once
#include <stddef.h>
#include <stdint.h>

void ui_transport_init(void);
int  ui_uart_read_bytes(uint8_t *buf, size_t max_len);
void ui_uart_send_bytes(const uint8_t *data, size_t len);
void ui_uart_rx_task(void *arg);
void ui_send_command(uint8_t cmd_id, uint16_t param16, uint32_t param32);

