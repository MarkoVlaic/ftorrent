#ifndef FTORRENT_PEER_ABSTRACT_PEER
#define FTORRENT_PEER_ABSTRACT_PEER

#include <vector>
#include <memory>
#include <cstdint>

#include "./messages.h"

namespace ftorrent {
namespace peer {
    struct AbstractPeer {
        virtual ~AbstractPeer() = default;

        virtual void message_handler(std::shared_ptr<messages::Message>) = 0;
        virtual void send_block(uint64_t piece_index, uint64_t block_offset, std::shared_ptr<std::vector<uint8_t>> data) = 0;
        virtual void send_have(uint64_t index) = 0;
        virtual bool has_piece(uint32_t index) = 0;
        virtual double get_download_rate() const = 0;
        virtual bool get_upload_interested() const = 0;
        virtual void choke_upload() = 0;
        virtual void unchoke_upload() = 0;
        virtual void close() = 0;
    };
}; // peer
}; // ftorrent

#endif
