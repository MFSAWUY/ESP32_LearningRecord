// SPDX-License-Identifier: MIT
// Original handbook teaching example; target: ESP32, ESP-IDF 6.1.
#include <stdint.h>
#include <math.h>
#include "driver/i2s_std.h"
#include "esp_log.h"
void app_main(void) {
    i2s_chan_handle_t rx;
    i2s_chan_config_t channel = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&channel, NULL, &rx));
    i2s_std_config_t cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {.mclk = I2S_GPIO_UNUSED, .bclk = 26, .ws = 25,
                     .dout = I2S_GPIO_UNUSED, .din = 33}
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx, &cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(rx));
    int32_t samples[512]; // stereo: select left words at indices 0,2,...
    while (1) {
        size_t received = 0;
        esp_err_t err = i2s_channel_read(rx, samples, sizeof(samples), &received, 1000);
        if (err != ESP_OK) { ESP_LOGW("audio", "read: %s", esp_err_to_name(err)); continue; }
        size_t frames = received / (2 * sizeof(int32_t));
        double sum = 0.0, sum_sq = 0.0;
        for (size_t i = 0; i < frames; ++i) {
            // INMP441-style 24 useful MSBs in 32-bit slot; selected L/R must be LOW.
            double x = samples[2 * i] / 2147483648.0;
            sum += x; sum_sq += x * x;
        }
        if (frames) {
            double mean = sum / frames;
            double variance = fmax(0.0, sum_sq / frames - mean * mean);
            double rms = sqrt(variance);
            ESP_LOGI("audio", "frames=%u rms=%.6f dBFS=%.2f",
                     (unsigned)frames, rms, 20.0 * log10(fmax(rms, 1e-12)));
        }
    }
}
