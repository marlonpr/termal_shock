#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include "ui_transport.h"
#include "ui_node.h"
#include "protocol.h"
#include "board_pins.h"

const char *TAG = "UI_MAIN";

/* ================= APP MAIN ================= */

void app_main(void)
{
    ESP_LOGI(TAG, "UI node starting");

    ui_transport_init();

    buttons_init();

    xTaskCreate(ui_uart_rx_task, "ui_uart_rx", 4096, NULL, 5, NULL);
    xTaskCreate(button_task, "button_task", 4096, NULL, 10, NULL);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


