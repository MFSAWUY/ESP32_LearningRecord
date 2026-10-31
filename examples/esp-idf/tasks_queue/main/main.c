// SPDX-License-Identifier: MIT
// Original handbook teaching example; target: ESP32, ESP-IDF 6.1.
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"
typedef struct { uint32_t sequence; int64_t generated_us; } sample_t;
static QueueHandle_t samples;
static void producer(void *arg) {
    (void)arg;
    uint32_t sequence = 0;
    TickType_t previous = xTaskGetTickCount();
    while (1) {
        sample_t item = {.sequence = ++sequence, .generated_us = esp_timer_get_time()};
        if (xQueueSend(samples, &item, 0) != pdTRUE)
            ESP_LOGW("producer", "queue full: newest sample dropped");
        vTaskDelayUntil(&previous, pdMS_TO_TICKS(100));
    }
}
static void consumer(void *arg) {
    (void)arg;
    sample_t item;
    while (1) {
        if (xQueueReceive(samples, &item, pdMS_TO_TICKS(1000)) == pdTRUE)
            ESP_LOGI("consumer", "seq=%" PRIu32 ", latency=%" PRId64 " us",
                     item.sequence, esp_timer_get_time() - item.generated_us);
        else ESP_LOGW("consumer", "producer timeout");
    }
}
void app_main(void) {
    samples = xQueueCreate(8, sizeof(sample_t));
    configASSERT(samples);
    BaseType_t a = xTaskCreate(producer, "producer", 3072, NULL, 5, NULL);
    BaseType_t b = xTaskCreate(consumer, "consumer", 3072, NULL, 4, NULL);
    configASSERT(a == pdPASS && b == pdPASS);
}
