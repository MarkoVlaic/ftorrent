#include <functional>
#include <iostream> // TODO remove

#include "service/peer/peer.h"
#include "service/peer/messages.h"
#include "service/types.h"
#include "service/util.h"

namespace ftorrent {
namespace peer {
    Peer::Peer(boost::asio::io_context& ioc, const tcp::resolver::results_type& eps, const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid, uint64_t num_pieces):
        peer_connection{ioc, eps, ih, pid, std::bind(&Peer::message_handler, this, std::placeholders::_1)},
        piece_present(num_pieces, false)
    {
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
                break;
            }

            case ftorrent::peer::messages::EMessageId::PIECE: {
                auto piece_ptr = std::static_pointer_cast<ftorrent::peer::messages::Piece>(msg_ptr);
                std::cerr << "piece: (index, begin) = (" << piece_ptr->index << ", " << piece_ptr->begin << ")\n";
                std::cerr << "block:\n";
                ftorrent::util::print_buffer(piece_ptr->block);
                break;
            }

            case ftorrent::peer::messages::EMessageId::CANCEL: {
                auto cancel_ptr = std::static_pointer_cast<ftorrent::peer::messages::Cancel>(msg_ptr);
                std::cerr << "request: (index, begin, length) = (" << cancel_ptr->index << ", " << cancel_ptr->begin << ", " << cancel_ptr->length << ")\n";
                break;
            }
        }
    }
};
};
