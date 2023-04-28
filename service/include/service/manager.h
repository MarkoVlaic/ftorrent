#ifndef ZAVRSNI_MANAGER_H
#define ZAVRSNI_MANAGER_H

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <string>
#include <array>
#include <iostream>
#include <memory>

#include "service/metainfo.h"
#include "service/torrent.h"
#include "service/tracker/tracker.h"
#include "service/tracker/udp_tracker.h"
#include "service/types.h"

namespace ftorrent {
    class Manager {
    public:
        Manager(const std::string& metainfo_path, const std::string& outfile_path, uint32_t nthreads):
        metainfo{metainfo_path},
        active_torrent{metainfo, outfile_path, io_context},
        num_threads{nthreads}, peer_id{generatePeerId()},
        port{ 6881 } /* TODO: assign available port */{
            initTracker();
        }

        void run();
    private:
        void initTracker();
    private:
        boost::asio::io_context io_context;
        boost::thread_group thread_pool;

        Metainfo metainfo;
        torrent::Torrent active_torrent;

        uint32_t num_threads;

        types::PeerId peer_id;
        uint16_t port;

        std::shared_ptr<tracker::Tracker> tracker;

        types::PeerId generatePeerId();
    };
}; // ftorrent

#endif
