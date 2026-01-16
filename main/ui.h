#pragma once

#include <stdint.h>
#include <stdbool.h>

/* ================= EVENT-DRIVEN UI ================= */

void ui_on_test_started(uint32_t max_cycles);
void ui_on_test_finished(uint32_t completed_cycles);
void ui_on_cycle_complete(uint32_t cycle_count);

/* ================= STATE / FLAGS ================= */

void ui_set_state(uint8_t state);
void ui_set_mode(uint8_t mode);

void ui_set_float_state(uint8_t index, bool active);

/* ================= TELEMETRY ================= */

void ui_set_temperature(uint8_t sensor, float temperature_c);
void ui_set_elapsed_seconds(uint32_t seconds);
void ui_set_cycle_count(uint32_t cycles);
