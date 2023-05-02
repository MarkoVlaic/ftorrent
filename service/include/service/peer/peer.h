#ifndef FTORRENT_PEER_H
#define FTORRENT_PEER_H

#include <boost/asio.hpp>
#include <memory>
#include <vector>

#include "./peer_connection.h"
#include "./messages.h"
#include "../types.h"

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

    class Peer {
    public:
        Peer(boost::asio::io_context& ioc, const tcp::resolver::results_type& eps, const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid, uint64_t num_pieces);

        void message_handler(std::shared_ptr<messages::Message>);
    private:
        PeerConnection peer_connection;

        DataChannel upload;
        DataChannel download;

        std::vector<bool> piece_present;
    };

};
};

#endif
