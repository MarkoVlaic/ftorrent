#ifndef FTORRENT_PEER_H
#define FTORRENT_PEER_H

#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <functional>

#include "./peer_connection.h"
#include "./messages.h"
#include "../types.h"
#include "./handler_types.h"

namespace ftorrent {
namespace peer {

    class DataChannel {
    public:
        void choke() {
            choked = true;
        }

        void unchoke() {
            choked = false;
        }

        bool is_choked() const {
            return choked;
        }

        void set_interested(bool i) {
            interested = i;
        }

        bool is_interested() const {
            return interested;
        }

    private:
        bool choked = true;
        bool interested = false;
    };

    class Peer : public std::enable_shared_from_this<Peer> {
    public:
        Peer(
            boost::asio::io_context& ioc, const tcp::resolver::results_type& eps,
            const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid,
            uint64_t num_pieces, ConnectionClosedHandler connection_closed,
            BlockRecievedHandler blk_rcvd, BlockRequestHandler blk_req
        );

        void message_handler(std::shared_ptr<messages::Message>);
        void send_block(uint64_t piece_index, uint64_t block_offset, std::shared_ptr<std::vector<uint8_t>> data);
        void send_have(uint64_t index);
    private:
        std::shared_ptr<PeerConnection> peer_connection;

        DataChannel upload;
        DataChannel download;

        std::vector<bool> piece_present;
        BlockRecievedHandler block_recieved;
        BlockRequestHandler block_requested;
    };

};
};

#endif
