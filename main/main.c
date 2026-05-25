#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "ui_transport.h"
#include "ui_node.h"
#include "led_panel.h"
#include "board_pins.h"
#include "driver/uart.h"
#include "esp_timer.h"

typedef enum {
    SM_IDLE,
    SM_PREHEAT,
    SM_HOT_DWELL,
    SM_COLD_DWELL,
    SM_WAIT
} sm_state_t;


typedef struct {
    sm_state_t state;
    uint32_t   elapsed;
    int64_t    sync_us;
} ui_state_t;

static ui_state_t ui;

bool parse_state_elapsed(const char *rx,
                         sm_state_t *out_state,
                         uint32_t *out_elapsed)
{
    int s;
    unsigned long e;

    if (sscanf(rx, "STATE=%d,ELAPSED=%lu", &s, &e) == 2) {
        *out_state   = (sm_state_t)s;
        *out_elapsed = (uint32_t)e;
        return true;
    }
    return false;
}



uint32_t ui_state_elapsed_now(void)
{
    int64_t delta_us =
        esp_timer_get_time() - ui.sync_us;

    return ui.elapsed + (delta_us / 1000000);
}




int t_cycles = 0;
int t_module = 0;
int mode = 0;
uint32_t cycles = 0;



bool parse_cycles(const char *rx, uint32_t *out_cycles)
{
    unsigned long v;

    if (sscanf(rx, "CYCLES=%lu", &v) == 1) {
        *out_cycles = (uint32_t)v;
        return true;
    }
    return false;
}



int parse_t1_to_int(const char *rx)
{
    float t1;

    if (sscanf(rx, "T1=%f", &t1) == 1) {
        return (int)(t1);   // fixed-point
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

        if (len <= 0) {
            continue;
        }

        buf[len] = '\0';
        ESP_LOGI("RX", "Received:\n%s", (char *)buf);

        char *line = strtok((char *)buf, "\n");
        while (line) {
	
			int t = parse_t1_to_int(line);
			if (t >= 0) {
			    t_module = t;
			    ESP_LOGI("PARSE", "T1 stored = %d", t_module);
			}
	
	
	        if (parse_cycles(line, &cycles)) {
				t_cycles = cycles;
	            ESP_LOGI("PARSE", "Cycles stored = %lu",
	                     (unsigned long)cycles);
	        }        
	        
	        if (parse_state_elapsed(line, &ui.state, &ui.elapsed)) {
		
			    ui.sync_us = esp_timer_get_time();
			
			    ESP_LOGI("PARSE",
			        "State=%d Elapsed=%lu",
			        ui.state,
			        (unsigned long)ui.elapsed
		    	);
			}			
	          
	
	        line = strtok(NULL, "\n");
        }
    }
}




void drawing_task(void *arg)
{

    int r=0; 
	int g=0; 
	int b=0;

    while (1)
    {
        clear_back_buffer();

    	if(t_module > 30)
		{
			r = 255;
			g = 0;
			b = 0;
		}
		else if(t_module < 30)
		{
			r = 0;
			g = 0;
			b = 255;		
		}

        // ---------------- DRAW ----------------
        char buf_temp[20];
        snprintf(buf_temp, sizeof(buf_temp), "T:%02d",   t_module);
        
        char buf_cycle[20];
        snprintf(buf_cycle, sizeof(buf_cycle), "C:%03d-250", t_cycles);		


/*
        char buf_mode[20];
		uint32_t elapsed_now = ui_state_elapsed_now();
		
		snprintf(buf_mode, sizeof(buf_mode),
		         "S:%02lu",
		         (unsigned long)elapsed_now);
*/
		         
        //snprintf(buf_mode, sizeof(buf_mode), "S:00",   t_module);


        draw_text(1, 43, buf_cycle, 0, 255, 0);

        draw_text(35, 10, buf_temp, r, g, b);
		//draw_text(3, 10, buf_mode, 255, 255, 255);
		draw_text(3, 10, "S:00", 255, 255, 255);

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
    xTaskCreate(button_task, "button_task", 4096, NULL, 4, NULL);
    
}



/*



bool parse_mode(const char *rx, int *out_mode)
{
    int v;

    if (sscanf(rx, "MODE=%d", &v) == 1) {
        *out_mode = (int)v;
        return true;
    }
    return false;
}
l







            if (parse_mode(line, &mode)) {
				//mode = mode;
                ESP_LOGI("PARSE", "Current Mode = %d",
                         mode);
            }
*/     