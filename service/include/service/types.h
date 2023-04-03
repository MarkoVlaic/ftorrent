#ifndef FTORRENT_TYPES_H
#define FTORRENT_TYPES_H

#include <array>

namespace ftorrent {
namespace types {
    using PeerId = std::array<uint8_t, 20>;
}; // types
}; // ftorrent

#endif
