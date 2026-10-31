#include <Arduino.h>
#include <cstdio>
#include <cstring>
#include "model.h"

namespace {
constexpr size_t kCommandCapacity = 64;
constexpr size_t kBytesPerLoop = 32;
char command[kCommandCapacity]{};
size_t command_length = 0;
bool discarding_line = false;
RunMode mode = RunMode::Running;
uint32_t period_ms = 1000;
uint32_t previous_ms = 0;
uint32_t sequence = 0;
Statistics statistics;

void print_help() {
    Serial.println("commands: help | status | pause | resume | reset | period N");
    Serial.println("N=50..60000 ms; newline required; samples are SYNTHETIC");
}

void print_status() {
    Serial.printf("status mode=%s period_ms=%lu count=%lu min_mv=%ld max_mv=%ld mean_mv=%.2f\n",
        mode == RunMode::Running ? "running" : "paused",
        static_cast<unsigned long>(period_ms),
        static_cast<unsigned long>(statistics.count()),
        static_cast<long>(statistics.minimum_mv()),
        static_cast<long>(statistics.maximum_mv()), statistics.mean_mv());
}

void handle_command(const char* text) {
    if (text[0] == '\0') return;
    if (std::strcmp(text, "help") == 0) {
        print_help();
    } else if (std::strcmp(text, "status") == 0) {
        print_status();
    } else if (std::strcmp(text, "pause") == 0) {
        mode = RunMode::Paused;
        Serial.println("ok paused");
    } else if (std::strcmp(text, "resume") == 0) {
        mode = RunMode::Running;
        previous_ms = millis();
        Serial.println("ok running");
    } else if (std::strcmp(text, "reset") == 0) {
        statistics.reset();
        sequence = 0;
        previous_ms = millis();
        Serial.println("ok statistics reset");
    } else if (std::strncmp(text, "period ", 7) == 0) {
        uint32_t candidate = period_ms;
        if (!parse_period(text + 7, candidate)) {
            Serial.println("error period must be decimal digits in 50..60000");
            return;
        }
        period_ms = candidate;
        previous_ms = millis();
        Serial.println("ok period updated");
    } else {
        Serial.println("error unknown command; use help");
    }
}

void read_commands() {
    for (size_t consumed = 0; consumed < kBytesPerLoop && Serial.available() > 0; ++consumed) {
        const int incoming = Serial.read();
        if (incoming < 0) break;
        const char c = static_cast<char>(incoming);
        if (c == '\r') continue;  // CRLF and LF are both accepted.
        if (c == '\n') {
            if (discarding_line) {
                Serial.println("error invalid or overlong command; entire line discarded");
            } else {
                command[command_length] = '\0';
                handle_command(command);
            }
            command_length = 0;
            discarding_line = false;
        } else if (!discarding_line) {
            const unsigned char byte = static_cast<unsigned char>(c);
            if (byte < 0x20U || byte > 0x7EU) {
                discarding_line = true;  // Reject embedded NUL and other binary bytes.
            } else if (command_length + 1 < kCommandCapacity) {
                command[command_length++] = c;
            } else {
                discarding_line = true;
            }
        }
    }
}
}  // namespace

void setup() {
    Serial.begin(115200);
    delay(250);  // Startup visibility only; ongoing work is non-blocking.
    Serial.println("language_basics source=synthetic (not ADC or sensor data)");
    Serial.printf("chip=%s revision=%u cores=%u flash_bytes=%lu heap_bytes=%lu\n",
        ESP.getChipModel(), static_cast<unsigned>(ESP.getChipRevision()),
        static_cast<unsigned>(ESP.getChipCores()),
        static_cast<unsigned long>(ESP.getFlashChipSize()),
        static_cast<unsigned long>(ESP.getFreeHeap()));
    print_help();
    previous_ms = millis();
}

void loop() {
    read_commands();
    const uint32_t now = millis();
    if (mode == RunMode::Running && uint32_t(now - previous_ms) >= period_ms) {
        previous_ms = now;  // No catch-up burst after a late iteration.
        Reading sample{now, 1000 + static_cast<int32_t>((sequence % 10U) * 100U), true};
        ++sequence;
        if (!statistics.add(sample)) {
            mode = RunMode::Paused;
            Serial.println("error statistics capacity reached; reset before resume");
            return;
        }
        char line[96];
        const int length = std::snprintf(line, sizeof(line), "sample,%lu,%ld,source=synthetic",
            static_cast<unsigned long>(sample.at_ms), static_cast<long>(sample.value_mv));
        if (length >= 0 && static_cast<size_t>(length) < sizeof(line)) {
            Serial.println(line);
        } else {
            Serial.println("error output buffer too small");
        }
    }
    delay(1);  // Yield briefly without a one-second response delay.
}
