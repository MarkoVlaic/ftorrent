#ifndef FTORRENT_PEER_H
#define FTORRENT_PEER_H

#include <boost/asio.hpp>
#include <memory>
#include <vector>
#include <functional>

#include "./abstract_peer.h"
#include "./peer_connection.h"
#include "./messages.h"
#include "../types.h"
#include "./handler_types.h"
#include "../piece_picker/piece_picker.h"
#include "../rate_measure.h"

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

        void data_transferred(uint64_t amount) {
            rate_measure.update(amount);
        }

        double get_data_rate() const {
            return rate_measure.get();
        }

    private:
        bool choked = true;
        bool interested = false;
        // TODO get this from config
        RateMeasure rate_measure{20.0};
    };

    class Peer : public AbstractPeer,  public std::enable_shared_from_this<Peer> {
    public:
        Peer(
            boost::asio::io_context& ioc, const tcp::resolver::results_type& eps,
            const ftorrent::types::Hash& ih, const ftorrent::types::PeerId& pid,
            uint64_t num_pieces, std::shared_ptr<ftorrent::piece_picker::PiecePicker> pc_pckr,
            ConnectionClosedHandler connection_closed, BlockRecievedHandler blk_rcvd, BlockRequestHandler blk_req
        );
        ~Peer() override = default;

        void message_handler(std::shared_ptr<messages::Message>) override;
        void send_block(uint64_t piece_index, uint64_t block_offset, std::shared_ptr<std::vector<uint8_t>> data) override;
        void send_have(uint64_t index) override;

        bool has_piece(uint32_t index) override;

        double get_download_rate() const override;
        bool get_upload_interested() const override;
        void choke_upload() override;
        void unchoke_upload() override;

        void close() override;
        void refresh_download_interest();
    private:
        void request_blocks();
        void cancel_block_requests();
    private:
        std::shared_ptr<PeerConnection> peer_connection;

        DataChannel upload;
        DataChannel download;

        std::vector<bool> piece_present;
        std::shared_ptr<ftorrent::piece_picker::PiecePicker> piece_picker;

        BlockRecievedHandler block_recieved;
        BlockRequestHandler block_requested;

        static constexpr uint32_t block_pipeline_size = 10;
        uint32_t blocks_in_queue = 0;
    };

};
};

#endif
