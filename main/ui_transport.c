// ui_transport.c
#include "ui_transport.h"
#include "driver/uart.h"
#include "board_pins.h"
#include "esp_log.h"
#include "crc16.h"


#include "ui_transport.h"
#include "protocol.h"

#define RX_BUF_SIZE       256
#define MAX_PAYLOAD_SIZE   64

void ui_uart_rx_task(void *arg)
{
    uint8_t rx_buf[RX_BUF_SIZE];
    size_t  rx_len = 0;

    while (1) {
        int n = ui_uart_read_bytes(rx_buf + rx_len, RX_BUF_SIZE - rx_len);
        if (n <= 0) continue;

        rx_len += n;

        while (rx_len >= sizeof(packet_header_t) + 2) { // header + CRC
            packet_header_t hdr;
            memcpy(&hdr, rx_buf, sizeof(hdr));

            if (hdr.magic != PACKET_MAGIC || hdr.version != PROTOCOL_VERSION) {
                memmove(rx_buf, rx_buf + 1, --rx_len);
                continue;
            }

            size_t total_len = sizeof(packet_header_t) + hdr.length + 2; // include CRC
            if (rx_len < total_len) break;

            uint16_t crc_recv = (rx_buf[total_len - 2] << 8) | rx_buf[total_len - 1];
            uint16_t crc_calc = crc16_ccitt(rx_buf, total_len - 2, 0);

            if (crc_calc != crc_recv) {
                ESP_LOGW("UI_UART_RX", "CRC fail, dropping byte");
                memmove(rx_buf, rx_buf + 1, --rx_len);
                continue;
            }

            // Handle STATUS packets
            if (hdr.type == PKT_STATUS) {
                payload_status_t status;
                memcpy(&status, rx_buf + sizeof(packet_header_t), sizeof(status));
                ESP_LOGI("UI_UART_RX",
                         "UI RX: T1=%.2f T2=%.2f T3=%.2f F1=%d F2=%d",
                         status.pt100_1_centi/100.0f,
                         status.pt100_2_centi/100.0f,
                         status.pt100_3_centi/100.0f,
                         status.float_mask & 0x01,
                         status.float_mask & 0x02
                );
            }

            // Consume packet
            memmove(rx_buf, rx_buf + total_len, rx_len - total_len);
            rx_len -= total_len;
        }
    }
}


void ui_send_command(uint8_t cmd_id, uint16_t param16, uint32_t param32)
{
    uint8_t buf[64];
    static uint32_t sequence = 1;

    size_t len = protocol_build_command_packet(buf, sizeof(buf),
                                               sequence++, cmd_id,
                                               param16, param32);
    if (len == 0) return;

    ui_uart_send_bytes(buf, len);
}

void ui_send_ack(uint32_t sequence, uint8_t cmd_id, uint8_t status)
{
    uint8_t buf[64];
    size_t len = protocol_build_ack_packet(buf, sizeof(buf), sequence, cmd_id, status);
    if (len > 0) {
        ui_uart_send_bytes(buf, len);
    }
}


static const char *TAG = "UI_UART";

#define RX_BUF_SIZE 256

void ui_transport_init(void)
{
    uart_config_t cfg = {
        .baud_rate  = 115200,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_UI, RX_BUF_SIZE, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_UI, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(UART_UI, PIN_UART_UI_TX, PIN_UART_UI_RX,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    ESP_LOGI(TAG, "UI UART initialized (TX=%d RX=%d)", PIN_UART_UI_TX, PIN_UART_UI_RX);
}

int ui_uart_read_bytes(uint8_t *buf, size_t max_len)
{
    return uart_read_bytes(UART_UI, buf, max_len, pdMS_TO_TICKS(100));
}

void ui_uart_send_bytes(const uint8_t *data, size_t len)
{
    uart_write_bytes(UART_UI, (const char *)data, len);
}
