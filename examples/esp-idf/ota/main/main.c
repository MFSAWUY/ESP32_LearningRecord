// SPDX-License-Identifier: MIT
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "nvs_flash.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "esp_app_desc.h"
#include "esp_crt_bundle.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_log.h"
#if __has_include("server_root.h")
#include "server_root.h"
#else
#include "server_root.example.h"
#endif
#define GOT_IP BIT0
#define WIFI_FAILED BIT1
static EventGroupHandle_t events;
static unsigned retries;
static void on_network(void *arg, esp_event_base_t base, int32_t id, void *data) {
    (void)arg; (void)data;
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) esp_wifi_connect();
    else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(events, GOT_IP);
        if (++retries <= 5) esp_wifi_connect();
        else xEventGroupSetBits(events, WIFI_FAILED);
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        retries = 0; xEventGroupSetBits(events, GOT_IP);
    }
}
static bool self_test(void) {
    // This demonstrates the contract. A product must test its critical devices.
#ifdef CONFIG_EXAMPLE_FORCE_SELFTEST_FAIL
    return false;
#else
    return heap_caps_check_integrity_all(true) &&
           heap_caps_get_free_size(MALLOC_CAP_8BIT) > 32768;
#endif
}
void app_main(void) {
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t state;
    ESP_LOGI("ota", "running version=%s partition=%s", esp_app_get_description()->version, running->label);
    if (esp_ota_get_state_partition(running, &state) == ESP_OK &&
        state == ESP_OTA_IMG_PENDING_VERIFY) {
        if (!self_test()) {
            ESP_LOGE("ota", "self-test rejected image");
            ESP_ERROR_CHECK(esp_ota_mark_app_invalid_rollback_and_reboot());
            return;
        }
        ESP_ERROR_CHECK(esp_ota_mark_app_valid_cancel_rollback());
        ESP_LOGI("ota", "new image confirmed");
    }
    if (!CONFIG_EXAMPLE_WIFI_SSID[0] || !CONFIG_EXAMPLE_OTA_URL[0]) {
        ESP_LOGW("ota", "configure SSID and HTTPS URL in menuconfig; no update attempted");
        return;
    }
    if (strncmp(CONFIG_EXAMPLE_OTA_URL, "https://", 8) != 0) {
        ESP_LOGE("ota", "HTTPS required"); return;
    }
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    configASSERT(esp_netif_create_default_wifi_sta());
    events = xEventGroupCreate();
    configASSERT(events);
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, on_network, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, on_network, NULL));
    wifi_init_config_t init = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init));
    wifi_config_t wifi = {0};
    if (strlen(CONFIG_EXAMPLE_WIFI_SSID) > sizeof(wifi.sta.ssid) ||
        strlen(CONFIG_EXAMPLE_WIFI_PASSWORD) > sizeof(wifi.sta.password)) {
        ESP_LOGE("ota", "SSID/password exceed Wi-Fi byte limits"); return;
    }
    memcpy(wifi.sta.ssid, CONFIG_EXAMPLE_WIFI_SSID, strlen(CONFIG_EXAMPLE_WIFI_SSID));
    memcpy(wifi.sta.password, CONFIG_EXAMPLE_WIFI_PASSWORD, strlen(CONFIG_EXAMPLE_WIFI_PASSWORD));
    wifi.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi));
    ESP_ERROR_CHECK(esp_wifi_start());
    EventBits_t bits = xEventGroupWaitBits(events, GOT_IP | WIFI_FAILED, pdFALSE, pdFALSE, pdMS_TO_TICKS(30000));
    if (!(bits & GOT_IP)) { ESP_LOGE("ota", "Wi-Fi timeout"); return; }
    esp_sntp_config_t clock = ESP_NETIF_SNTP_DEFAULT_CONFIG(CONFIG_EXAMPLE_SNTP_SERVER);
    ESP_ERROR_CHECK(esp_netif_sntp_init(&clock));
    if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(30000)) != ESP_OK) {
        ESP_LOGE("ota", "clock not synchronized; TLS validation needs real time"); return;
    }
    esp_http_client_config_t http = {.url = CONFIG_EXAMPLE_OTA_URL, .timeout_ms = 10000,
        .keep_alive_enable = true};
    if (SERVER_ROOT_PEM[0]) http.cert_pem = SERVER_ROOT_PEM;
    else http.crt_bundle_attach = esp_crt_bundle_attach;
    esp_https_ota_config_t options = {.http_config = &http};
    esp_https_ota_handle_t update = NULL;
    esp_err_t err = esp_https_ota_begin(&options, &update);
    if (err != ESP_OK) { ESP_LOGE("ota", "begin: %s", esp_err_to_name(err)); return; }
    esp_app_desc_t incoming;
    err = esp_https_ota_get_img_desc(update, &incoming);
    if (err != ESP_OK ||
        strncmp(incoming.version, esp_app_get_description()->version, sizeof(incoming.version)) == 0) {
        ESP_LOGW("ota", "header invalid or version unchanged; update skipped");
        esp_https_ota_abort(update); return;
    }
    // A maximum session length prevents an indefinitely stalled experiment.
    int64_t deadline = esp_timer_get_time() + 180LL * 1000000;
    do {
        err = esp_https_ota_perform(update);
        if (esp_timer_get_time() > deadline) { err = ESP_ERR_TIMEOUT; break; }
    } while (err == ESP_ERR_HTTPS_OTA_IN_PROGRESS);
    if (err != ESP_OK || !esp_https_ota_is_complete_data_received(update)) {
        ESP_LOGE("ota", "download failed: %s; keep old boot image", esp_err_to_name(err));
        esp_https_ota_abort(update); return;
    }
    err = esp_https_ota_finish(update);
    if (err == ESP_OK) {
        ESP_LOGI("ota", "verified image written; rebooting");
        esp_restart();
    } else ESP_LOGE("ota", "finish: %s", esp_err_to_name(err));
}
