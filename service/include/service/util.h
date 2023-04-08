#ifndef FTORRENT_UTIL_H
#define FTORRENT_UTIL_H

#include <random>
#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdint>
#include <functional>
#include <vector>

namespace ftorrent {
namespace util {
    class RandomByteGenerator {
    public:
        static RandomByteGenerator& getInstance() {
            static RandomByteGenerator instance;
            return instance;
        }

        template<typename ForwardIterator>
        void generate(ForwardIterator first, ForwardIterator last) {
            std::generate(first, last, std::ref(rbe));
        }

        std::vector<uint8_t> generate(uint32_t num) {
            std::vector<uint8_t> result;
            result.resize(num);

            generate(result.begin(), result.end());
            return result;
        }

        RandomByteGenerator(RandomByteGenerator const&) = delete;
        void operator=(RandomByteGenerator const&) = delete;
        RandomByteGenerator(RandomByteGenerator&&) = delete;
        RandomByteGenerator& operator=(RandomByteGenerator&& other) = delete;
    private:
        RandomByteGenerator() {
            uint32_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
            rbe.seed(seed);
        }

        std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char> rbe;
    };

    std::array<uint32_t, 2> splitUint64t(uint64_t num);

}; // util
}; // ftorrent

#endif
