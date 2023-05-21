#ifndef FTORRENT_RATE_MEASURE
#define FTORRENT_RATE_MEASURE

#include <chrono>

namespace ftorrent {

    class RateMeasure {
    public:
        RateMeasure(double rate_period);
        void update(uint64_t amount);

        inline double get() const {
            return rate;
        }
    private:
        using duration = std::chrono::duration<double>;
        using time_point = std::chrono::time_point<std::chrono::steady_clock, duration>;

        time_point rate_since;
        time_point last;
        duration max_rate_period;
        uint64_t total = 0;
        double rate = 0.0;
    };
};

#endif
