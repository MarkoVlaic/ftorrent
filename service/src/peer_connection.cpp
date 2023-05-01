#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/endian.hpp>
#include <memory>
#include <algorithm>
#include <iostream> // TODO: remove

#include "service/peer/peer_connection.h"
#include "service/util.h"
#include "service/peer/errors.h"

namespace ftorrent {
namespace peer {
    PeerConnection::PeerConnection(boost::asio::io_context& ioc, const tcp::resolver::results_type& eps, const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid, std::function<void(std::shared_ptr<messages::Message>)> msg_hdlr):
    io_context{ioc}, send_strand{boost::asio::make_strand(ioc)},
    socket{ioc}, endpoints{eps},
    info_hash{ih}, peer_id{pid}, message_handler{msg_hdlr} {
        std::cout << "make connect\n";
        boost::asio::async_connect(socket, endpoints, [this](const boost::system::error_code& e, tcp::endpoint) {
            std::cout << "connected\n";
            this->connected(e);
        });
    }

    void PeerConnection::connected(const boost::system::error_code& error) {
        if(error) {
            throw HandshakeError{"Connect failed"};
            return;
        }

        // start the handshake
        std::cout << "connected handler\n";

        auto handshake = std::make_shared<messages::Handshake>("BitTorrent protocol", info_hash, peer_id);
        ftorrent::util::print_buffer(info_hash);
        send(handshake, [this]() {
            handshake_state.sent = true;

            if(handshake_state.complete()) {
                handshake_complete();
            }
            std::cout << "handler lambda\n";
        });

        recieve_handshake();
    }

    void PeerConnection::send(std::shared_ptr<messages::Message> msg) {
        send(msg, [](){});
    }

    void PeerConnection::send(std::shared_ptr<messages::Message> msg, std::function<void()> handler) {
        std::cout << "send\n";

        if(!handshake_state.complete() && msg->id != messages::EMessageId::HANDSHAKE) {
            auto pending = std::make_pair(msg, handler);
            pending_send_msgs.push_back(pending);
            return;
        }

        ftorrent::serialization::Serializer serializer;
        msg->serialize(serializer);

        std::vector<uint8_t> buf(serializer.data());
        send_buffers.push_back(buf);
        std::size_t buf_index = send_buffers.size() - 1;

        std::cout << "out buf\n";
        ftorrent::util::print_buffer(buf);

        boost::asio::async_write(
            socket,
            boost::asio::buffer(send_buffers[buf_index]),
            boost::asio::bind_executor(send_strand, [&, handler, this](const boost::system::error_code& err, long unsigned int) {
                if(err) {
                    // TODO: signal connection closed
                    return;
                }

                send_buffers.erase(send_buffers.begin() + buf_index);
                handler();
                //std::cout << "fire handler\n";
            })
        );
    }

    void PeerConnection::recieve(uint32_t bytes, std::function<void()> handler) {
        recv_buffer.resize(bytes);
        boost::asio::async_read(
            socket, boost::asio::buffer(recv_buffer, bytes),
            [&, handler](const boost::system::error_code& e, long unsigned int){
                if(e) {
                    // TODO: signal recieve error
                    return;
                }

                handler();
            }
        );
    }

    void PeerConnection::recieve_handshake() {
        std::cout << "recv handshake\n";
        recieve(1, [this](){
            std::cout << "1 byte\n";
            uint8_t pstrlen = recv_buffer[0];
            recieve(pstrlen + 48, [&, this]() {
                std::vector<uint8_t> msg_data(recv_buffer);
                msg_data.insert(msg_data.begin(), pstrlen);

                std::cout << "message data:\n" << std::hex;
                for(auto byte : msg_data) {
                    std::cout << (int)byte << " ";
                }
                std::cout << "\n";


                messages::Handshake handshake;
                ftorrent::serialization::Deserializer deserializer{msg_data};
                ftorrent::serialization::deserialize(handshake, deserializer);

                std::cout << "desered:\n";
                std::cout << handshake.pstr << "\n";
                ftorrent::util::print_buffer(handshake.info_hash);
                ftorrent::util::print_buffer(handshake.peer_id);

                if(!std::equal(info_hash.begin(), info_hash.end(), handshake.info_hash.begin())) {
                    // TODO: signal wrong info hash
                    std::cout << "info hash mismatch, got ih bytes:\n" << std::endl;
                    for(auto byte : handshake.info_hash) {
                        std::cout << (int)byte << " ";
                    }
                    std::cout << "\n";

                    return;
                }

                handshake_state.recieved = true;
                if(handshake_state.complete()) {
                    handshake_complete();
                }
            });
        });
    }

    void PeerConnection::handshake_complete() {
        std::cout << "handshake complete\n";
        for(auto pending : pending_send_msgs) {
            send(pending.first, pending.second);
        }
        recieve_message();
    }

    void PeerConnection::recieve_message() {
        recieve(4, [this](){
            uint32_t len;
            std::vector<uint8_t> len_bytes = recv_buffer;

            ftorrent::serialization::Deserializer deserializer{recv_buffer};
            ftorrent::serialization::deserialize(len, deserializer);
            len = boost::endian::big_to_native(len);

            std::cout << "got len " << std::dec << len << std::endl;
            if(len == 0) {
                // KEEP-ALIVE
                recieve_message();
                return;
            }

            recieve(len, [len, len_bytes, this](){
                messages::EMessageId id = static_cast<messages::EMessageId>(recv_buffer[0]);
                std::cerr << "got id " << id << std::endl;

                std::vector<uint8_t> msg_bytes;
                msg_bytes.resize(4 + len);
                std::copy(len_bytes.begin(), len_bytes.end(), msg_bytes.begin());
                std::copy(recv_buffer.begin(), recv_buffer.end(), msg_bytes.begin() + 4);

                std::cerr << "message bytes:";
                ftorrent::util::print_buffer(msg_bytes);

                ftorrent::serialization::Deserializer deserializer{msg_bytes};
                std::shared_ptr<messages::Message> result;

                switch(id) {
                    case messages::EMessageId::CHOKE:
                        std::cerr << "choke\n";
                        result = std::make_shared<messages::Choke>();
                        break;
                    case messages::EMessageId::UNCHOKE:
                        std::cerr << "UNCHOKE\n";
                        result = std::make_shared<messages::Unchoke>();
                        break;
                    case messages::EMessageId::INTERESTED:
                        std::cerr << "INTERESTED\n";
                        result = std::make_shared<messages::Interested>();
                        break;
                    case messages::EMessageId::NOT_INTERESTED:
                        std::cerr << "NOT INTERESTED\n";
                        result = std::make_shared<messages::NotInterested>();
                        break;
                    case messages::EMessageId::HAVE: {
                        std::cerr << "HAVE\n";
                        result = std::make_shared<messages::Have>();
                        ftorrent::serialization::deserialize(*std::static_pointer_cast<messages::Have>(result), deserializer);
                        break;
                    }
                    case messages::EMessageId::BITFIELD: {
                        std::cerr << "BITFIELD\n";
                        result = std::make_shared<messages::BitField>();
                        ftorrent::serialization::deserialize(*std::static_pointer_cast<messages::BitField>(result), deserializer);
                        break;
                    }
                    case messages::EMessageId::REQUEST: {
                        std::cerr << "REQUEST\n";
                        result = std::make_shared<messages::Request>();
                        ftorrent::serialization::deserialize(*std::static_pointer_cast<messages::Request>(result), deserializer);
                        break;
                    }
                    case messages::EMessageId::PIECE: {
                        std::cerr << "PIECE\n";
                        result = std::make_shared<messages::Piece>();
                        ftorrent::serialization::deserialize(*std::static_pointer_cast<messages::Piece>(result), deserializer);
                        break;
                    }
                    case messages::EMessageId::CANCEL: {
                        std::cerr << "CANCEL\n";
                        result = std::make_shared<messages::Cancel>();
                        ftorrent::serialization::deserialize(*std::static_pointer_cast<messages::Cancel>(result), deserializer);
                        break;
                    }
                    default:
                        std::cerr << "???\n";
                }

                message_handler(result);
                recieve_message();
            });
        });
    }

}; // peer
}; // ftorrent
