#include "model.h"
#include <cmath>
#include <cstdio>
#include <limits>

#define REQUIRE(condition) do { \
    if (!(condition)) { \
        std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); \
        return 1; \
    } \
} while (false)

int main() {
    uint32_t period = 1000;
    REQUIRE(parse_period("50", period) && period == 50);
    REQUIRE(parse_period("60000", period) && period == 60000);
    REQUIRE(parse_period("000250", period) && period == 250);

    const char* invalid[] = {nullptr, "", "49", "60001", "-50", "+50",
        "50x", " 50", "50 ", "50.0", "9999999999999999999999999999999999"};
    for (const char* text : invalid) {
        period = 1234;
        REQUIRE(!parse_period(text, period));
        REQUIRE(period == 1234);  // A rejected command must not change the configuration.
    }

    Statistics statistics;
    REQUIRE(statistics.count() == 0 && statistics.mean_mv() == 0.0);
    REQUIRE(!statistics.add(Reading{0, 999, false}));
    REQUIRE(statistics.count() == 0);
    REQUIRE(statistics.add(Reading{1, 1000, true}));
    REQUIRE(statistics.add(Reading{2, -500, true}));
    REQUIRE(statistics.add(Reading{3, 2500, true}));
    REQUIRE(statistics.count() == 3);
    REQUIRE(statistics.minimum_mv() == -500 && statistics.maximum_mv() == 2500);
    REQUIRE(std::abs(statistics.mean_mv() - 1000.0) < 1e-12);

    statistics.reset();
    const int32_t high = std::numeric_limits<int32_t>::max();
    const int32_t low = std::numeric_limits<int32_t>::min();
    REQUIRE(statistics.add(Reading{0, high, true}));
    REQUIRE(statistics.add(Reading{1, high, true}));  // Detect a wrongly narrowed 32-bit sum.
    REQUIRE(statistics.mean_mv() == static_cast<double>(high));
    REQUIRE(statistics.add(Reading{2, low, true}));
    REQUIRE(statistics.minimum_mv() == low);
    REQUIRE(statistics.maximum_mv() == high);
    const double expected = (2.0 * high + low) / 3.0;
    REQUIRE(std::abs(statistics.mean_mv() - expected) < 1e-6);

    statistics.reset();
    REQUIRE(statistics.count() == 0);
    REQUIRE(statistics.minimum_mv() == 0 && statistics.maximum_mv() == 0);
    REQUIRE(statistics.mean_mv() == 0.0);
    std::puts("PASS: parser bounds/rejection, unchanged configuration, statistics, wide sum, reset");
    return 0;
}
