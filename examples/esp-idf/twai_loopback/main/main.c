// SPDX-License-Identifier: MIT
// Original handbook teaching example; target: ESP32, ESP-IDF 6.1.
#include <inttypes.h>
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_log.h"
typedef struct { uint32_t id; uint8_t data[8]; uint8_t size; } received_t;
static QueueHandle_t queue;
static bool on_rx(twai_node_handle_t node, const twai_rx_done_event_data_t *event, void *ctx) {
    (void)event; (void)ctx;
    received_t out = {0};
    twai_frame_t frame = {.buffer = out.data, .buffer_len = sizeof(out.data)};
    BaseType_t wake = pdFALSE;
    if (twai_node_receive_from_isr(node, &frame) == ESP_OK) {
        out.id = frame.header.id; out.size = frame.header.dlc;
        xQueueSendFromISR(queue, &out, &wake);
    }
    return wake == pdTRUE;
}
void app_main(void) {
    queue = xQueueCreate(8, sizeof(received_t));
    configASSERT(queue);
    twai_node_handle_t node;
    twai_onchip_node_config_t cfg = {
        .io_cfg.tx = 25, .io_cfg.rx = 26, .bit_timing.bitrate = 125000,
        .tx_queue_depth = 4, .fail_retry_cnt = 0,
        .flags.enable_self_test = true, .flags.enable_loopback = true
    };
    ESP_ERROR_CHECK(twai_new_node_onchip(&cfg, &node));
    twai_event_callbacks_t callbacks = {.on_rx_done = on_rx};
    ESP_ERROR_CHECK(twai_node_register_event_callbacks(node, &callbacks, NULL));
    ESP_ERROR_CHECK(twai_node_enable(node));
    uint8_t data[2] = {0x42, 0};
    twai_frame_t frame = {.header.id = 0x123, .buffer = data, .buffer_len = sizeof(data)};
    while (1) {
        ++data[1];
        ESP_ERROR_CHECK(twai_node_transmit(node, &frame, 100));
        // Keep frame and buffer alive until DMA/controller completes.
        esp_err_t err = twai_node_transmit_wait_all_done(node, 1000);
        if (err != ESP_OK) {
            ESP_LOGE("twai", "TX timeout; stop before reusing buffer");
            ESP_ERROR_CHECK(twai_node_disable(node)); break;
        }
        received_t out;
        if (xQueueReceive(queue, &out, pdMS_TO_TICKS(1000)))
            ESP_LOGI("twai", "id=%" PRIx32 " size=%u counter=%u", out.id, out.size, out.data[1]);
        else ESP_LOGW("twai", "no loopback; check GPIO25--GPIO26 jumper");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ESP_ERROR_CHECK(twai_node_delete(node));
    vQueueDelete(queue);
}
