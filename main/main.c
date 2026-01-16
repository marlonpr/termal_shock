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


int phase = 0;
int mode = 0;
int local_counter = 0;
int cycle = 0;






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
        snprintf(buf_time, sizeof(buf_time), "%d",   local_counter);        
        
        

        
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

    
    
    
    
      //  xTaskCreate(ui_uart_rx_task, "ui_uart_rx", 4096, NULL, 2, NULL);


    
    

   // xTaskCreate(ui_uart_rx_task, "ui_uart_rx", 4096, NULL, 3, NULL);
    xTaskCreate(button_task, "button_task", 4096, NULL, 4, NULL);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


