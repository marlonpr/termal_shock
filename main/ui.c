#include "ui.h"
#include "esp_log.h"

static const char *TAG = "UI";

/* ================= EVENTS ================= */

void ui_on_test_started(uint32_t max_cycles)
{
    ESP_LOGI(TAG, "UI: Test started (max_cycles=%lu)", max_cycles);
}

void ui_on_test_finished(uint32_t completed_cycles)
{
    ESP_LOGI(TAG, "UI: Test finished (cycles=%lu)", completed_cycles);
}

void ui_on_cycle_complete(uint32_t cycle_count)
{
    ESP_LOGI(TAG, "UI: Cycle complete (%lu)", cycle_count);
}

/* ================= STATE ================= */

void ui_set_state(uint8_t state)
{
    ESP_LOGI(TAG, "UI: State=%u", state);
}

void ui_set_mode(uint8_t mode)
{
    ESP_LOGI(TAG, "UI: Mode=%u", mode);
}

void ui_set_float_state(uint8_t index, bool active)
{
    ESP_LOGI(TAG, "UI: Float %u = %s", index, active ? "ON" : "OFF");
}

/* ================= TELEMETRY ================= */

void ui_set_temperature(uint8_t sensor, float temperature_c)
{
    ESP_LOGI(TAG, "UI: T[%u]=%.2f C", sensor, temperature_c);
}

void ui_set_elapsed_seconds(uint32_t seconds)
{
    ESP_LOGI(TAG, "UI: Elapsed=%lu s", seconds);
}

void ui_set_cycle_count(uint32_t cycles)
{
    ESP_LOGI(TAG, "UI: Cycles=%lu", cycles);
}
