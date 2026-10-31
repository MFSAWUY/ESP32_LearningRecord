// SPDX-License-Identifier: MIT
// Original handbook teaching example; target: ESP32, ESP-IDF 6.1.
#include <inttypes.h>
#include "esp_sleep.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
RTC_DATA_ATTR static uint32_t wake_count;
void app_main(void) {
    ESP_LOGI("sleep", "RTC count=%" PRIu32 ", wake cause=%d",
             ++wake_count, (int)esp_sleep_get_wakeup_cause());
    // Deep sleep restarts the application. RTC data is not a durable database.
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(10ULL * 1000000ULL));
    vTaskDelay(pdMS_TO_TICKS(1000)); // allow serial log to drain
    esp_deep_sleep_start();
}
