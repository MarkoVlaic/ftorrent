#include <boost/bind.hpp>
#include <algorithm>
#include <random>
#include <unistd.h>
#include <chrono>
#include <memory>

#include "service/manager.h"
#include "service/util.h"
#include "service/tracker/udp_tracker.h"

#include "service/peer/peer_connection.h" // TODO: remove

namespace ftorrent {
    void Manager::run() {
        // TODO uncomment
        tracker->start();

        for(int i=0;i<num_threads;i++) {
            thread_pool.create_thread(
                boost::bind(&boost::asio::io_context::run, &io_context)
            );
        }


        // peer test
        boost::asio::ip::tcp::resolver resolver{io_context};
        auto endpoints = resolver.resolve("127.0.0.1", "51413");
        peer::PeerConnection peer_conn{io_context, endpoints, metainfo.info_hash, peer_id};

        thread_pool.join_all();
    }

    types::PeerId Manager::generatePeerId() {
        types::PeerId result;
        char start[] = "ftorrent";

        std::copy(start, start + 8, result.begin());
        uint32_t pid = getpid();
        for(int i=0;i<4;i++) {
            result[8 + i] = (uint8_t)((pid >> i*8) & 0xF);
        }

        util::RandomByteGenerator& generator = util::RandomByteGenerator::getInstance();
        generator.generate(result.begin() + 10, result.end());

        return result;
    }

    void Manager::initTracker() {
        std::string announce_url = metainfo.announce;
        int proto_separator = announce_url.find("://");
        std::string protocol = announce_url.substr(0, proto_separator);
        std::string rest = announce_url.substr(proto_separator + 3);

        if(protocol == "udp") {
            int serv_separator = rest.find(":");
            std::string host = rest.substr(0, serv_separator);
            std::string service = rest.substr(serv_separator + 1);

            std::cout << "host " << host << " service " << service << "\n";

            tracker = std::make_shared<tracker::UdpTracker>(io_context, host, service, metainfo.info_hash, peer_id, 6881);

            return;
        }

        std::string error = protocol;
        error += " protocol is not supported";
        throw ftorrent::Exception{error};
    }
};
