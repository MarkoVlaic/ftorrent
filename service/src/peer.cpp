#include <functional>
#include <iostream> // TODO remove

#include "service/peer/peer.h"
#include "service/peer/messages.h"
#include "service/types.h"
#include "service/util.h"
#include "service/piece_picker/piece_picker.h"

namespace ftorrent {
namespace peer {
    Peer::Peer(
        boost::asio::io_context& ioc, const tcp::resolver::results_type& eps,
        const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid,
        uint64_t num_pieces, std::shared_ptr<ftorrent::piece_picker::PiecePicker> pc_pckr,
        ConnectionClosedHandler connection_closed, BlockRecievedHandler blk_rcvd, BlockRequestHandler blk_req
    ):
        peer_connection{std::make_shared<PeerConnection>(
            ioc, eps, ih, pid,
            std::bind(&Peer::message_handler, this, std::placeholders::_1),
            [connection_closed, this]() { connection_closed(shared_from_this()); }
        )},
        piece_present(num_pieces, false), piece_picker{pc_pckr},
        block_recieved{blk_rcvd}, block_requested{blk_req}
    {
        std::cerr << "init peer conn ptr " << peer_connection << "\n";
        auto bitfield_msg = std::make_shared<messages::BitField>(piece_picker->get_have_bitfield());
        peer_connection->send(bitfield_msg);
    }

    void Peer::message_handler(std::shared_ptr<messages::Message> msg_ptr) {
        std::cerr << "got message id: " << (int) msg_ptr->id << "\n";

        switch(msg_ptr->id) {
            case ftorrent::peer::messages::EMessageId::CHOKE: {
                download.choke();
                //cancel_block_requests();
                break;
            }

            case ftorrent::peer::messages::EMessageId::UNCHOKE: {
                std::cerr << "UNCHOKE\n";
                download.unchoke();
                request_blocks();
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
                std::cerr << "got have= " << have_ptr->index << "\n";
                piece_present[have_ptr->index] = true;
                piece_picker->on_have(have_ptr->index);

                refresh_download_interest();
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

                refresh_download_interest();
                piece_picker->on_bitfield(bitfield_ptr->bitfield);
                break;
            }

            case ftorrent::peer::messages::EMessageId::REQUEST: {
                auto req_ptr = std::static_pointer_cast<ftorrent::peer::messages::Request>(msg_ptr);
                std::cerr << "request: (index, begin, length) = (" << req_ptr->index << ", " << req_ptr->begin << ", " << req_ptr->length << ")\n";

                block_requested(shared_from_this(), req_ptr->index, req_ptr->begin, req_ptr->length);
                break;
            }

            case ftorrent::peer::messages::EMessageId::PIECE: {
                auto piece_msg = std::static_pointer_cast<ftorrent::peer::messages::Piece>(msg_ptr);
                std::cerr << "piece: (index, begin) = (" << piece_msg->index << ", " << piece_msg->begin << ")\n";
                std::cerr << "block:\n";
                ftorrent::util::print_buffer(piece_msg->block);

                auto piece = piece_picker->get_piece(piece_msg->index);
                auto block = piece->get_block_by_offset(piece_msg->begin);
                block->lock();
                block->set_requested(false);
                block->unlock();
                blocks_in_queue--;

                block_recieved(piece_msg->index, piece_msg->begin, piece_msg->block);
                request_blocks();
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

    bool Peer::has_piece(uint32_t index) {
        return piece_present[index];
    }

    void Peer::refresh_download_interest() {
        std::vector<bool> have_pieces = piece_picker->get_have_bitfield();
        bool interesting = false;
        for(int i=0;i<have_pieces.size();i++) {
            if(!have_pieces[i] && piece_present[i]) {
                if(!download.is_interested()) {
                    peer_connection->send(std::make_shared<messages::Interested>());
                    download.set_interested(true);
                }

                interesting = true;
            }
        }

        if(!interesting) {
            download.set_interested(false);
            peer_connection->send(std::make_shared<messages::NotInterested>());
        }
    }

    void Peer::request_blocks() {
        int attempt = 0;
        while(blocks_in_queue < block_pipeline_size) {
            std::shared_ptr<Piece> piece = piece_picker->next(shared_from_this(), attempt);
            std::cerr << "got piece\n";
            if(piece == nullptr) {
                break;
            }

            for(auto block : piece->blocks) {
                if(!block->requested() && !block->complete()) {
                    if(block->try_lock()) {
                        block->set_requested(true);
                        block->unlock();

                        auto block_req = std::make_shared<messages::Request>(piece->index, block->offset, block->size);
                        std::cerr << "request index = " << piece->index << " offset " << block->offset << " size " << block->size << "\n";
                        peer_connection->send(block_req);

                        blocks_in_queue++;
                    }
                }
            }

            attempt++;
        }
    }
};
};
