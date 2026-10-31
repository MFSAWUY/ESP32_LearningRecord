// SPDX-License-Identifier: MIT
// Original handbook teaching example; target: ESP32, ESP-IDF 6.1.
#include <inttypes.h>
#include "esp_chip_info.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void app_main(void) {
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    ESP_LOGI("diag", "IDF=%s cores=%d reset_reason=%d",
             esp_get_idf_version(), chip.cores, (int)esp_reset_reason());
    while (1) {
        ESP_LOGI("diag", "uptime_ms=%" PRId64 " free=%u minimum=%u largest=%u",
                 esp_timer_get_time() / 1000,
                 (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
                 (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
                 (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
        ESP_LOGI("diag", "task stack high water=%u bytes",
                 (unsigned)uxTaskGetStackHighWaterMark(NULL));
        configASSERT(heap_caps_check_integrity_all(true));
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
