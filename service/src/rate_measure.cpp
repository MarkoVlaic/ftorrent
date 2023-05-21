#include <chrono>

#include "service/rate_measure.h"

namespace ftorrent {
    RateMeasure::RateMeasure(double rate_period):
        rate_since{std::chrono::steady_clock::now()}, last{rate_since},
        max_rate_period{rate_period}
    {}

    void RateMeasure::update(uint64_t amount) {
        double prev_amount = (last - rate_since).count() * rate;
        time_point t = std::chrono::steady_clock::now();
        rate = (prev_amount + amount)/(t - rate_since).count();
        last = t;

        if(rate_since < t - max_rate_period) {
            auto new_rate_since =  t - max_rate_period;
            auto offset = new_rate_since - rate_since;
            rate_since += offset;
        }
    }
};
