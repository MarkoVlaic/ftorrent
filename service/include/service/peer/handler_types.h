#ifndef FTORRENT_PEER_HANDLER_TYPES_H
#define FTORRENT_PEER_HANDLER_TYPES_H

#include <memory>
#include <functional>
#include <vector>
#include <cstdint>

namespace ftorrent {
namespace peer {
    class Peer;
    using ConnectionClosedHandler = std::function<void(std::shared_ptr<Peer>)>;
    using BlockRecievedHandler = std::function<void(uint64_t, uint64_t, std::vector<uint8_t>&)>;
    using BlockRequestHandler = std::function<void(std::shared_ptr<Peer>, uint32_t, uint32_t, uint32_t)>;
}; // peer
}; // ftorrent

#endif
