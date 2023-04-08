#include "service/util.h"

namespace ftorrent {
namespace util {
    std::array<uint32_t, 2> splitUint64t(uint64_t num) {
        uint32_t word_mask = 0xFFFFFFFF;
        std::array<uint32_t, 2> parts = { ((unsigned int)(num >> 8)) & word_mask, (unsigned int)(num & word_mask) };

        return parts;
    }
};
};
