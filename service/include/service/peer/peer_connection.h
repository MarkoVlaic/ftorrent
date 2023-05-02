#ifndef FTORRENT_PEER_CONNECTION_H
#define FTORRENT_PEER_CONNECTION_H

#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <utility>

#include "../types.h"
#include "messages.h"

namespace ftorrent {
namespace peer {
    using boost::asio::ip::tcp;

    struct HandshakeState {
        bool sent = false;
        bool recieved = false;

        bool complete() {
            return sent && recieved;
        }
    };

    class PeerConnection {
    public:
        PeerConnection(boost::asio::io_context& ioc, const tcp::resolver::results_type& eps, const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid, std::function<void(std::shared_ptr<messages::Message>)> msg_hdlr);
        PeerConnection(PeerConnection&&) = default;

        void send(std::shared_ptr<messages::Message> msg);
        void send(std::shared_ptr<messages::Message> msg, std::function<void()> handler);

    private:
        void connected(const boost::system::error_code& error);

        void recieve(uint32_t bytes, std::function<void()> handler);

        void handshake_sent(const boost::system::error_code& error);
        void handshake_recieved(const boost::system::error_code& error);
        void recieve_handshake();
        void handshake_complete();

        void recieve_message();
    private:
        boost::asio::io_context& io_context;
        boost::asio::strand<boost::asio::io_context::executor_type> send_strand;
        tcp::socket socket;
        const tcp::resolver::results_type& endpoints;

        ftorrent::types::Hash info_hash;
        ftorrent::types::PeerId peer_id;

        std::function<void(std::shared_ptr<messages::Message>)> message_handler;

        HandshakeState handshake_state;

        std::vector<std::vector<uint8_t>> send_buffers;
        std::vector<uint8_t> recv_buffer;
        std::vector<std::pair<std::shared_ptr<messages::Message>, std::function<void()>>> pending_send_msgs;
    };
}; // peer
}; // ftorrent

#endif

