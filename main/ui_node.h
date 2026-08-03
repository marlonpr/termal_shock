// ui_node.h
#pragma once
#include <stdint.h>
#include <stddef.h>

extern const char *TAG;


void button_task(void *arg);
void buttons_init(void);

extern volatile uint32_t  t_max_cycles;


