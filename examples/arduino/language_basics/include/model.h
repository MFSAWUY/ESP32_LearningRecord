#ifndef HANDBOOK_MODEL_H
#define HANDBOOK_MODEL_H

#include <cstdint>

// Fixed-width fields and explicit units keep the model independent of Arduino.
struct Reading {
    uint32_t at_ms;
    int32_t value_mv;
    bool valid;
};

enum class RunMode : uint8_t { Running, Paused };

// On error, output_ms is left unchanged. Accepted input: decimal digits only,
// with the value in [50, 60000]. No signs, whitespace or trailing characters.
bool parse_period(const char* text, uint32_t& output_ms);

class Statistics {
public:
    bool add(const Reading& reading);
    void reset();
    uint32_t count() const { return count_; }
    int32_t minimum_mv() const { return minimum_mv_; }
    int32_t maximum_mv() const { return maximum_mv_; }
    double mean_mv() const;

private:
    uint32_t count_ = 0;
    int64_t sum_mv_ = 0;
    int32_t minimum_mv_ = 0;
    int32_t maximum_mv_ = 0;
};

#endif
