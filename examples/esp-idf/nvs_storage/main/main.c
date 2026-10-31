// SPDX-License-Identifier: MIT
// Original handbook teaching example; target: ESP32, ESP-IDF 6.1.
#include <inttypes.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
void app_main(void) {
    esp_err_t err = nvs_flash_init();
    if (err != ESP_OK) {
        ESP_LOGE("nvs", "init: %s; preserve data, inspect partition before erase",
                 esp_err_to_name(err)); return;
    }
    nvs_handle_t handle;
    ESP_ERROR_CHECK(nvs_open("handbook", NVS_READWRITE, &handle));
    uint32_t boots = 0;
    err = nvs_get_u32(handle, "boots", &boots);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE("nvs", "read: %s", esp_err_to_name(err)); nvs_close(handle); return;
    }
    if (boots != UINT32_MAX) ++boots;
    err = nvs_set_u32(handle, "boots", boots);
    if (err == ESP_OK) err = nvs_commit(handle);
    if (err == ESP_OK) ESP_LOGI("nvs", "persistent boot count=%" PRIu32, boots);
    else ESP_LOGE("nvs", "write/commit: %s", esp_err_to_name(err));
    nvs_close(handle);
}
