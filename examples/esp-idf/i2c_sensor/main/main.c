// SPDX-License-Identifier: MIT
// Original handbook teaching example; target: ESP32, ESP-IDF 6.1.
#include <stdint.h>
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
static uint8_t crc8(const uint8_t *data, size_t length) {
    uint8_t crc = 0xff;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (unsigned b = 0; b < 8; ++b)
            crc = (uint8_t)((crc & 0x80) ? (crc << 1) ^ 0x31 : crc << 1);
    }
    return crc;
}
void app_main(void) {
    i2c_master_bus_handle_t bus;
    i2c_master_bus_config_t cfg = {.i2c_port = I2C_NUM_0,
        .sda_io_num = 21, .scl_io_num = 22, .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7, .flags.enable_internal_pullup = true};
    ESP_ERROR_CHECK(i2c_new_master_bus(&cfg, &bus));
    i2c_device_config_t dev_cfg = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x44, .scl_speed_hz = 100000};
    i2c_master_dev_handle_t sensor;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &dev_cfg, &sensor));
    const uint8_t measure[] = {0x24, 0x00}; // high repeatability, no clock stretching
    while (1) {
        uint8_t data[6];
        esp_err_t err = i2c_master_transmit(sensor, measure, sizeof(measure), 100);
        if (err == ESP_OK) {
            vTaskDelay(pdMS_TO_TICKS(20));
            err = i2c_master_receive(sensor, data, sizeof(data), 100);
        }
        if (err != ESP_OK) ESP_LOGW("sht30", "read failed: %s", esp_err_to_name(err));
        else if (crc8(data, 2) != data[2] || crc8(data + 3, 2) != data[5])
            ESP_LOGW("sht30", "CRC rejected; no measurement published");
        else {
            uint16_t t = ((uint16_t)data[0] << 8) | data[1];
            uint16_t h = ((uint16_t)data[3] << 8) | data[4];
            ESP_LOGI("sht30", "temperature=%.2f C humidity=%.2f %%RH",
                     -45.0 + 175.0 * t / 65535.0, 100.0 * h / 65535.0);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
