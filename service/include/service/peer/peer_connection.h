#ifndef FTORRENT_PEER_CONNECTION_H
#define FTORRENT_PEER_CONNECTION_H

#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <utility>
#include <chrono>
#include <mutex>
#include <iostream> // TODO remove

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

    class PeerConnection : public std::enable_shared_from_this<PeerConnection> {
    public:
        PeerConnection(
            boost::asio::io_context& ioc, const tcp::resolver::results_type& eps,
            const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid,
            std::function<void(std::shared_ptr<messages::Message>)> msg_hdlr,
            std::function<void()> conn_closed_hdlr
        );

        PeerConnection(
            boost::asio::io_context& ioc, boost::asio::ip::tcp::socket sock,
            const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid,
            std::function<void(std::shared_ptr<messages::Message>)> msg_hdlr,
            std::function<void()> conn_closed_hdlr
        );

        ~PeerConnection() {
            std::cerr << "DESTRUCTOR!\n";
        }

        void start();

        void send(std::shared_ptr<messages::Message> msg);
        void send(std::shared_ptr<messages::Message> msg, std::function<void()> handler);

    private:

        void recieve(uint32_t bytes, std::function<void()> handler);

        void handshake_sent(const boost::system::error_code& error);
        void handshake_recieved(const boost::system::error_code& error);
        void recieve_handshake();
        void handshake_complete();

        void recieve_message();

    private:
        class KeepAliveTimer {
        public:
            KeepAliveTimer(PeerConnection&, uint32_t);
            void reset();
            void stop();

        private:
            PeerConnection& connection;
            std::chrono::seconds interval; // in seconds
            boost::asio::steady_timer timer;

            void handler(const boost::system::error_code&);
        };

    private:
        boost::asio::io_context& io_context;
        boost::asio::strand<boost::asio::io_context::executor_type> send_strand;
        tcp::socket socket;

        ftorrent::types::Hash info_hash;
        ftorrent::types::PeerId peer_id;

        std::function<void(std::shared_ptr<messages::Message>)> message_handler;
        std::function<void()> connection_closed_handler;

        HandshakeState handshake_state;
        KeepAliveTimer keep_alive_timer;

        std::vector<std::shared_ptr<std::vector<uint8_t>>> send_buffers;
        std::mutex send_mutex;
        std::vector<uint8_t> recv_buffer;
        std::vector<std::pair<std::shared_ptr<messages::Message>, std::function<void()>>> pending_send_msgs;
    };
}; // peer
}; // ftorrent

#endif

