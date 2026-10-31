// SPDX-License-Identifier: MIT
// Original handbook teaching example; target: ESP32, ESP-IDF 6.1.
#include <stdint.h>
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void app_main(void) {
    uart_config_t config = {.baud_rate = 19200, .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, .source_clk = UART_SCLK_DEFAULT};
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 1024, 1024, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &config));
    // TX17 -> DI, RX16 <- RO, RTS27 -> DE and /RE on a 3.3V transceiver.
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 17, 16, 27, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_mode(UART_NUM_2, UART_MODE_RS485_HALF_DUPLEX));
    uint8_t bytes[128];
    while (1) {
        int count = uart_read_bytes(UART_NUM_2, bytes, sizeof(bytes), pdMS_TO_TICKS(500));
        if (count > 0) {
            ESP_LOGI("rs485", "received %d bytes; echo after sender releases bus", count);
            vTaskDelay(pdMS_TO_TICKS(10));
            int sent = uart_write_bytes(UART_NUM_2, bytes, count);
            esp_err_t err = uart_wait_tx_done(UART_NUM_2, pdMS_TO_TICKS(100));
            if (sent != count || err != ESP_OK) ESP_LOGW("rs485", "incomplete transmit");
        }
    }
}
