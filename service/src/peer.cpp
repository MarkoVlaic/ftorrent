#include <functional>
#include <iostream> // TODO remove

#include "service/peer/peer.h"
#include "service/peer/messages.h"
#include "service/types.h"
#include "service/util.h"

namespace ftorrent {
namespace peer {
    Peer::Peer(
        boost::asio::io_context& ioc, const tcp::resolver::results_type& eps,
        const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid,
        uint64_t num_pieces, ConnectionClosedHandler connection_closed,
        BlockRecievedHandler blk_rcvd, BlockRequestHandler blk_req
    ):
        peer_connection{std::make_shared<PeerConnection>(
            ioc, eps, ih, pid,
            std::bind(&Peer::message_handler, this, std::placeholders::_1),
            [connection_closed, this]() { connection_closed(shared_from_this()); }
        )},
        piece_present(num_pieces, false), block_recieved{blk_rcvd}, block_requested{blk_req}
    {
        std::cerr << "init peer conn ptr " << peer_connection << "\n";
    }

    void Peer::message_handler(std::shared_ptr<messages::Message> msg_ptr) {
        std::cerr << "got message id: " << (int) msg_ptr->id << "\n";

        switch(msg_ptr->id) {
            case ftorrent::peer::messages::EMessageId::CHOKE: {
                download.choke();
                break;
            }

            case ftorrent::peer::messages::EMessageId::UNCHOKE: {
                download.unchoke();
                break;
            }

            case ftorrent::peer::messages::EMessageId::INTERESTED: {
                upload.set_interested(true);
                break;
            }

            case ftorrent::peer::messages::EMessageId::NOT_INTERESTED: {
                upload.set_interested(false);
                break;
            }

            case ftorrent::peer::messages::EMessageId::HAVE: {
                auto have_ptr = std::static_pointer_cast<ftorrent::peer::messages::Have>(msg_ptr);
                std::cerr << "index = " << have_ptr->index << "\n";
                piece_present[have_ptr->index] = true;
                break;
            }

            case ftorrent::peer::messages::EMessageId::BITFIELD: {
                // TODO drop connection on invalid bitfield
                auto bitfield_ptr = std::static_pointer_cast<ftorrent::peer::messages::BitField>(msg_ptr);
                std::cerr << "bitfield: ";
                for(int i=0;i<piece_present.size();i++) {
                    std::cerr << bitfield_ptr->bitfield[i] << " ";
                    piece_present[i] = piece_present[i] || bitfield_ptr->bitfield[i];
                }
                std::cerr << "\n";
                break;
            }

            case ftorrent::peer::messages::EMessageId::REQUEST: {
                auto req_ptr = std::static_pointer_cast<ftorrent::peer::messages::Request>(msg_ptr);
                std::cerr << "request: (index, begin, length) = (" << req_ptr->index << ", " << req_ptr->begin << ", " << req_ptr->length << ")\n";

                block_requested(shared_from_this(), req_ptr->index, req_ptr->begin, req_ptr->length);
                break;
            }

            case ftorrent::peer::messages::EMessageId::PIECE: {
                auto piece_ptr = std::static_pointer_cast<ftorrent::peer::messages::Piece>(msg_ptr);
                std::cerr << "piece: (index, begin) = (" << piece_ptr->index << ", " << piece_ptr->begin << ")\n";
                std::cerr << "block:\n";
                ftorrent::util::print_buffer(piece_ptr->block);

                block_recieved(piece_ptr->index, piece_ptr->begin, piece_ptr->block);
                break;
            }

            case ftorrent::peer::messages::EMessageId::CANCEL: {
                auto cancel_ptr = std::static_pointer_cast<ftorrent::peer::messages::Cancel>(msg_ptr);
                std::cerr << "request: (index, begin, length) = (" << cancel_ptr->index << ", " << cancel_ptr->begin << ", " << cancel_ptr->length << ")\n";
                break;
            }
        }
    }

    void Peer::send_block(uint64_t piece_index, uint64_t block_offset, std::shared_ptr<std::vector<uint8_t>> data) {
        std::cerr << "send block\ndata:";
        util::print_buffer(*data);
        auto msg = std::make_shared<messages::Piece>(piece_index, block_offset, *data);
        std::cerr << "before send ptr is " << peer_connection << "\n";
        peer_connection->send(msg, [](){ std::cerr << "block sent\n"; });
        std::cerr << "after send\n";
    }

    void Peer::send_have(uint64_t index) {
        auto msg = std::make_shared<messages::Have>(index);
        peer_connection->send(msg);
    }
};
};
