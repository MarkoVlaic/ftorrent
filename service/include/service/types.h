#ifndef FTORRENT_TYPES_H
#define FTORRENT_TYPES_H

#include <array>
#include <cstdint>

namespace ftorrent {
namespace types {
    using PeerId = std::array<uint8_t, 20>;
    using Hash = std::array<uint8_t, 20>;

    struct PeerDescriptor {
        PeerDescriptor(uint32_t i, uint16_t p): ip{i}, port{p} {}

        uint32_t ip;
        uint16_t port;
    };
}; // types
}; // ftorrent

#endif
