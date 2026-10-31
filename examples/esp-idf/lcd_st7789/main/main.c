// SPDX-License-Identifier: MIT
#include <stdint.h>
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static bool sent(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *event, void *ctx) {
    (void)io; (void)event;
    BaseType_t wake = pdFALSE;
    vTaskNotifyGiveFromISR((TaskHandle_t)ctx, &wake);
    return wake == pdTRUE;
}
void app_main(void) {
    spi_bus_config_t bus = {.mosi_io_num = 23, .miso_io_num = -1, .sclk_io_num = 18,
        .quadwp_io_num = -1, .quadhd_io_num = -1, .max_transfer_sz = 240 * 16 * 2};
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO));
    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_io_spi_config_t io_cfg = {.cs_gpio_num = 13, .dc_gpio_num = 21,
        .spi_mode = 0, .pclk_hz = 20000000, .trans_queue_depth = 4,
        .lcd_cmd_bits = 8, .lcd_param_bits = 8};
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &io_cfg, &io));
    esp_lcd_panel_io_callbacks_t callbacks = {.on_color_trans_done = sent};
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io, &callbacks, xTaskGetCurrentTaskHandle()));
    esp_lcd_panel_handle_t panel;
    esp_lcd_panel_dev_config_t panel_cfg = {.reset_gpio_num = 27,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, .bits_per_pixel = 16};
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io, &panel_cfg, &panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel, true));
    uint16_t *pixels = heap_caps_malloc(240 * 16 * sizeof(uint16_t), MALLOC_CAP_DMA);
    configASSERT(pixels);
    const uint16_t rgb565[] = {0xf800, 0x07e0, 0x001f};
    unsigned color = 0;
    while (1) {
        uint16_t c = rgb565[color++ % 3];
        // SPI sends bytes in address order; RGB565 pixels need MSB first.
        uint16_t swapped = (uint16_t)((c >> 8) | (c << 8));
        for (unsigned i = 0; i < 240 * 16; ++i) pixels[i] = swapped;
        for (int y = 0; y < 240; y += 16) {
            ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, 0, y, 240, y + 16, pixels));
            // Buffer may not be changed or freed until asynchronous DMA completes.
            if (!ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1000))) {
                ESP_LOGE("lcd", "DMA timeout, stop instead of reusing buffer");
                vTaskSuspend(NULL);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
