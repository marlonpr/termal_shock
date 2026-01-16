#pragma once
#include <stdint.h>
#include <stddef.h>

void ui_rx_log_init(void);
void ui_rx_log_push(const uint8_t *data, uint16_t len);
