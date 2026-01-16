#include "ui_rx_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <string.h>

#define RX_LOG_QUEUE_LEN  16
#define RX_LOG_MAX_BYTES 64

static const char *TAG = "UI_RX_LOG";

typedef struct {
    uint16_t len;
    uint8_t  data[RX_LOG_MAX_BYTES];
} rx_log_msg_t;

static QueueHandle_t rx_log_q;

static void ui_rx_log_task(void *arg)
{
    rx_log_msg_t msg;

    ESP_LOGI(TAG, "RX log task started");

    while (1) {
        if (xQueueReceive(rx_log_q, &msg, portMAX_DELAY)) {
            ESP_LOGI(TAG, "RX packet (%u bytes)", msg.len);
            ESP_LOG_BUFFER_HEXDUMP(TAG, msg.data, msg.len, ESP_LOG_INFO);
        }
    }
}

void ui_rx_log_init(void)
{
    rx_log_q = xQueueCreate(RX_LOG_QUEUE_LEN, sizeof(rx_log_msg_t));
    configASSERT(rx_log_q);

    xTaskCreate(
        ui_rx_log_task,
        "ui_rx_log",
        3072,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL
    );
}

void ui_rx_log_push(const uint8_t *data, uint16_t len)
{
    if (!rx_log_q || !data || len == 0)
        return;

    rx_log_msg_t msg;
    msg.len = (len > RX_LOG_MAX_BYTES) ? RX_LOG_MAX_BYTES : len;
    memcpy(msg.data, data, msg.len);

    xQueueSend(rx_log_q, &msg, 0);  // non-blocking
}
