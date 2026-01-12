#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "ui_transport.h"
#include "ui_node.h"
#include "protocol.h"
#include "board_pins.h"


/* ================= CONFIG ================= */

#define LONG_PRESS_MS   300
#define POLL_MS         10

/* ================= TYPES ================= */

typedef enum {
    BTN_START = 0,
    BTN_PAUSE,
    BTN_RESET,
    BTN_COUNT
} button_t;

typedef struct {
    TickType_t press_tick;
    bool pending;
} btn_state_t;

/* ================= GLOBALS ================= */

static QueueHandle_t button_queue;
static btn_state_t btn_state[BTN_COUNT];

/* ================= ISR ================= */

static void IRAM_ATTR button_isr(void *arg)
{
    button_t btn = (button_t)(uint32_t)arg;
    BaseType_t hp = pdFALSE;
    xQueueSendFromISR(button_queue, &btn, &hp);
    if (hp) portYIELD_FROM_ISR();
}

/* ================= BUTTON INIT ================= */

void buttons_init(void)
{
    gpio_config_t io = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
        .pin_bit_mask =
            (1ULL << PIN_START) |
            (1ULL << PIN_PAUSE) |
            (1ULL << PIN_RESET)
    };
    gpio_config(&io);

    button_queue = xQueueCreate(8, sizeof(button_t));

    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_START, button_isr, (void *)BTN_START);
    gpio_isr_handler_add(PIN_PAUSE, button_isr, (void *)BTN_PAUSE);
    gpio_isr_handler_add(PIN_RESET, button_isr, (void *)BTN_RESET);

    ESP_LOGI(TAG, "Buttons initialized");
}

/* ================= BUTTON → COMMAND ================= */

static void handle_button(button_t btn)
{
    switch (btn) {

    case BTN_START:
        ESP_LOGI(TAG, "START button");
        ui_send_command(CMD_START_TEST, 0, 0);
        break;

    case BTN_PAUSE:
        ESP_LOGI(TAG, "PAUSE button");
        ui_send_command(CMD_NOP, 1, 0);   // param16=1 → pause
        break;

    case BTN_RESET:
        ESP_LOGI(TAG, "RESET button");
        ui_send_command(CMD_STOP_TEST, 0, 0);
        break;

    default:
        break;
    }
}

/* ================= BUTTON TASK ================= */

void button_task(void *arg)
{
    button_t btn;

    while (1) {

        if (xQueueReceive(button_queue, &btn, pdMS_TO_TICKS(POLL_MS))) {
            if (!btn_state[btn].pending) {
                btn_state[btn].press_tick = xTaskGetTickCount();
                btn_state[btn].pending = true;
            }
        }

        TickType_t now = xTaskGetTickCount();

        for (int i = 0; i < BTN_COUNT; i++) {

            if (!btn_state[i].pending)
                continue;

            gpio_num_t pin =
                (i == BTN_START) ? PIN_START :
                (i == BTN_PAUSE) ? PIN_PAUSE :
                                   PIN_RESET;

            /* Released too early */
            if (gpio_get_level(pin)) {
                btn_state[i].pending = false;
                continue;
            }

            /* Long press confirmed */
            if ((now - btn_state[i].press_tick) >=
                pdMS_TO_TICKS(LONG_PRESS_MS)) {

                handle_button((button_t)i);
                btn_state[i].pending = false;

                /* Wait for release */
                while (!gpio_get_level(pin)) {
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
        }
    }
}