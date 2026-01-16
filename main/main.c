#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "ui_rx_log.h"
#include "ui_transport.h"
#include "ui_node.h"
#include "ui.h"
#include "led_panel.h"
#include "board_pins.h"
#include "driver/uart.h"


int phase = 0;
int mode = 0;
int local_counter = 0;
int cycle = 0;


int t_module = 0;

int parse_t1_to_int(const char *rx)
{
    float t1;

    if (sscanf(rx, "T1=%f", &t1) == 1) {
        return (int)(t1 * 100.0f);   // fixed-point
    }

    return -1;  // parse error
}


void uart_rx_task(void *arg)
{
    uint8_t buf[256];

    while (1) {
        int len = uart_read_bytes(
            UART_UI,
            buf,
            sizeof(buf) - 1,
            pdMS_TO_TICKS(1000)
        );

        if (len > 0) {
            buf[len] = '\0';  // make it a string
            ESP_LOGI("RX", "Received: \"%s\"", (char *)buf);
        }
        
        if (len > 0) {
    buf[len] = '\0';

    int t1_int = parse_t1_to_int((char *)buf);

    if (t1_int >= 0) {
        ESP_LOGI("PARSE", "T1 stored as int = %d", t1_int/100);
        t_module = t1_int/100;
    }
}

    }
}




void drawing_task(void *arg)
{


    int r, g, b;

    while (1)
    {
        clear_back_buffer();

        // ---------------- STATE MACHINE ----------------
        if (phase == 0)   // long count: 0–90
        {
            if (local_counter > 90)
            {
                local_counter = 1;
                phase = 1;
            }
        }
        else              // short count: 0–30
        {
            if (local_counter > 30)
            {
                local_counter = 1;
                phase = 0;

                if (mode == 0)
                {
                    mode = 1;
                }
                else
                {
                    mode = 0;
                    cycle++;    // completed full cycle
                }
            }
        }

        // ---------------- COLOR LOGIC ----------------
        if (phase == 1)
        {
            // Short count → WHITE
            r = 255;
            g = 255;
            b = 255;
        }
        else
        {
            // Long count → depends on mode
            if (mode == 0)
            {
                // Mode 0 → RED
                r = 255;
                g = 0;
                b = 0;
            }
            else
            {
                // Mode 1 → BLUE
                r = 0;
                g = 0;
                b = 255;
            }
        }

        // ---------------- DRAW ----------------
        char buf_time[20];
        snprintf(buf_time, sizeof(buf_time), "%d",   t_module);        
        
        

        
        char buf_cycle[20];
        snprintf(buf_cycle, sizeof(buf_cycle), "C:%03d-500", cycle);

        draw_text(40, 10, buf_time, r, g, b);
        
        draw_text(1, 43, buf_cycle, 0, 255, 0);

        swap_buffers();
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}












const char *TAG = "UI_MAIN";

/* ================= APP MAIN ================= */

void app_main(void)
{
    ESP_LOGI(TAG, "UI node starting");

    ui_transport_init();

    buttons_init();
    
    
    
    
    
    //    ui_rx_log_init();

    
    
    
    
    
    
    
    init_pins();

	init_oe_pwm();           // initialize OE PWM
	//set_global_brightness(100);  // 50% brightness

	//init_nvs_brightness();

	init_planes();

    // Clear both buffers first time
    memset((void*)fbA, 0, sizeof(fbA));
    memset((void*)fbB, 0, sizeof(fbB));

    // Start refresh task (pin-driving) on core 0
	xTaskCreatePinnedToCore(refresh_task, "refresh_task", 2048, NULL, 1, NULL, 0);
	xTaskCreatePinnedToCore(drawing_task, "DrawTime", 4096, NULL, 1, NULL, 1);

    
    
    
    
       xTaskCreatePinnedToCore(uart_rx_task, "ui_uart_rx", 4096, NULL, 2, NULL,1);


    
    

   // xTaskCreate(ui_uart_rx_task, "ui_uart_rx", 4096, NULL, 3, NULL);
    xTaskCreate(button_task, "button_task", 4096, NULL, 4, NULL);

    //xTaskCreate(uart_rx_task, "uart_rx", 2048, NULL, 10, NULL);



    }



