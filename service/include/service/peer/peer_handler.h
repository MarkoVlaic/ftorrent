#ifndef FTORRENT_PEER_HANDLER_H
#define FTORRENT_PEER_HANDLER_H

#include <vector>
#include <boost/asio.hpp>
#include <memory>

#include "./peer.h"
#include "../types.h"

namespace ftorrent {
namespace peer {
    using boost::asio::ip::tcp;
    class PeerHandler {
    public:
        PeerHandler(
            boost::asio::io_context& ioc, types::Hash h, types::PeerId pid,
            uint64_t np, uint16_t lp,
            BlockRecievedHandler blk_rcvd,  BlockRequestHandler blk_req
        );

        void add_peer(types::PeerDescriptor pd);
        void piece_complete(uint32_t piece_index);
    private:
        void remove_peer(std::shared_ptr<Peer>);
    private:
        boost::asio::io_context& io_context;
        types::Hash info_hash;
        types::PeerId peer_id;
        uint64_t num_pieces;

        uint16_t listen_port;
        tcp::acceptor acceptor;
        std::vector<tcp::endpoint> local_endpoints;
        std::vector<std::shared_ptr<Peer>> peers;


        BlockRecievedHandler block_recieved;
        BlockRequestHandler block_requested;
    };
}; // peer
}; // ftorrent

#endif
