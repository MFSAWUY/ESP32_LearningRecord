#include "model.h"

#include <cerrno>
#include <cstdlib>
#include <limits>

bool parse_period(const char* text, uint32_t& output_ms) {
    if (text == nullptr || text[0] == '\0') return false;
    // Reject signs and spaces explicitly: strtoul alone would accept them.
    for (const char* cursor = text; *cursor != '\0'; ++cursor) {
        if (*cursor < '0' || *cursor > '9') return false;
    }
    errno = 0;
    char* end = nullptr;
    const unsigned long parsed = std::strtoul(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0') return false;
    if (parsed < 50UL || parsed > 60000UL) return false;
    output_ms = static_cast<uint32_t>(parsed);
    return true;
}

bool Statistics::add(const Reading& reading) {
    if (!reading.valid || count_ == std::numeric_limits<uint32_t>::max()) {
        return false;
    }
    if (count_ == 0) {
        minimum_mv_ = maximum_mv_ = reading.value_mv;
    } else {
        if (reading.value_mv < minimum_mv_) minimum_mv_ = reading.value_mv;
        if (reading.value_mv > maximum_mv_) maximum_mv_ = reading.value_mv;
    }
    // At most UINT32_MAX signed 32-bit readings fit in signed 64-bit sum.
    sum_mv_ += static_cast<int64_t>(reading.value_mv);
    ++count_;
    return true;
}

void Statistics::reset() {
    count_ = 0;
    sum_mv_ = 0;
    minimum_mv_ = maximum_mv_ = 0;
}

double Statistics::mean_mv() const {
    return count_ == 0 ? 0.0 : static_cast<double>(sum_mv_) / count_;
}
