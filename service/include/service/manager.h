#ifndef ZAVRSNI_MANAGER_H
#define ZAVRSNI_MANAGER_H

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <string>
#include <array>
#include <iostream>
#include <memory>
#include <cstdint>
#include <vector>

#include "service/metainfo.h"
#include "service/torrent.h"
#include "service/tracker/tracker.h"
#include "service/tracker/udp_tracker.h"
#include "service/types.h"
#include "service/peer/peer_handler.h"
#include "service/peer/peer.h"
#include "service/piece.h"

namespace ftorrent {
    class Manager {
    public:
        Manager(const std::string& metainfo_path, const std::string& outfile_path, uint32_t nthreads);

        void run();
    private:
        void init_tracker();
        types::PeerId generate_peer_id();
        std::vector<std::shared_ptr<Piece>> generate_pieces();

        void handle_block_request(std::shared_ptr<peer::Peer>, uint32_t, uint32_t, uint32_t);
    private:
        boost::asio::io_context io_context;
        boost::thread_group thread_pool;

        Metainfo metainfo;
        std::vector<std::shared_ptr<Piece>> pieces;
        Torrent active_torrent;

        uint32_t num_threads;

        types::PeerId peer_id;
        uint16_t port;

        std::shared_ptr<tracker::Tracker> tracker;
        peer::PeerHandler peer_handler;

    };
}; // ftorrent

#endif
