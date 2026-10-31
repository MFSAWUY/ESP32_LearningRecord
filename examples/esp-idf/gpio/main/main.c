// SPDX-License-Identifier: MIT
// Original handbook teaching example; target: ESP32, ESP-IDF 6.1.
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#define LED GPIO_NUM_25
#define BUTTON GPIO_NUM_27
void app_main(void) {
    gpio_config_t out = {.pin_bit_mask = 1ULL << LED, .mode = GPIO_MODE_OUTPUT};
    gpio_config_t in = {.pin_bit_mask = 1ULL << BUTTON, .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE};
    ESP_ERROR_CHECK(gpio_config(&out));
    ESP_ERROR_CHECK(gpio_config(&in));
    int previous = 1, stable = 1, led = 0;
    TickType_t changed = xTaskGetTickCount();
    while (1) {
        int raw = gpio_get_level(BUTTON);
        TickType_t now = xTaskGetTickCount();
        if (raw != previous) { previous = raw; changed = now; }
        if (raw != stable && (now - changed) >= pdMS_TO_TICKS(30)) {
            stable = raw;
            if (stable == 0) {
                led = !led;
                ESP_ERROR_CHECK(gpio_set_level(LED, led));
                ESP_LOGI("gpio", "pressed, led=%d", led);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
