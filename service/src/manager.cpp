#include <boost/bind.hpp>
#include <algorithm>
#include <random>
#include <unistd.h>
#include <chrono>
#include <memory>

#include "service/manager.h"
#include "service/util.h"
#include "service/tracker/udp_tracker.h"

#include "service/peer/peer.h" // TODO: remove

namespace ftorrent {
    Manager::Manager(const std::string& metainfo_path, const std::string& outfile_path, uint32_t nthreads):
        metainfo{metainfo_path},
        pieces{generate_pieces()},
        active_torrent{
            metainfo, outfile_path, io_context, pieces,
            [this](uint32_t i){ peer_handler.piece_complete(i); }
        },
        num_threads{nthreads}, peer_id{generate_peer_id()},
        port{ 6881 }, /* TODO: assign available port */
        peer_handler{
            io_context, metainfo.info_hash, peer_id, pieces, port,
            [this](uint64_t i, uint64_t o, std::vector<uint8_t>& d){
                active_torrent.write_block(i, o, d);
            },
            std::bind(&Manager::handle_block_request, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)
        }
    {
        std::cerr << "piece size " << metainfo.piece_length << "\n";
        init_tracker();
    }

    void Manager::run() {
        // TODO uncomment
        tracker->start();

        // peer test
        /*
        boost::asio::ip::tcp::resolver resolver{io_context};
        auto endpoints = resolver.resolve("127.0.0.1", "51413");
        peer::Peer peer{io_context, endpoints, metainfo.info_hash, peer_id, metainfo.pieces.size()};
        */

        //types::PeerDescriptor pd{2130706433, 50007};
        //peer_handler.add_peer(pd);

        for(int i=0;i<num_threads;i++) {
            thread_pool.create_thread(
                boost::bind(&boost::asio::io_context::run, &io_context)
            );
        }

        thread_pool.join_all();
    }

    types::PeerId Manager::generate_peer_id() {
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

    void Manager::init_tracker() {
        std::string announce_url = metainfo.announce;
        int proto_separator = announce_url.find("://");
        std::string protocol = announce_url.substr(0, proto_separator);
        std::string rest = announce_url.substr(proto_separator + 3);

        auto on_peers = [this](std::vector<types::PeerDescriptor> peers) {
            for(auto peer : peers) {
                peer_handler.add_peer(peer);
            }
        };

        if(protocol == "udp") {
            int serv_separator = rest.find(":");
            std::string host = rest.substr(0, serv_separator);
            std::string service = rest.substr(serv_separator + 1);

            std::cout << "host " << host << " service " << service << "\n";

            tracker = std::make_shared<tracker::UdpTracker>(io_context, host, service, metainfo.info_hash, peer_id, 6881, on_peers);

            return;
        }

        std::string error = protocol;
        error += " protocol is not supported";
        throw ftorrent::Exception{error};
    }

    std::vector<std::shared_ptr<Piece>> Manager::generate_pieces() {
        std::vector<std::shared_ptr<Piece>> pieces;
        pieces.reserve(metainfo.pieces.size());
        uint32_t index=0;
        for(auto it=metainfo.pieces.begin();it!=metainfo.pieces.end();it++) {
            uint32_t  s;
            if(std::distance(it, metainfo.pieces.end()) == 1) {
                s = metainfo.length - (metainfo.pieces.size() - 1) * metainfo.piece_length;
            } else {
                s = metainfo.piece_length;
            }

            pieces.push_back(std::make_shared<Piece>(index, *it, s));
            index++;
        }

        return pieces;
    }

    void Manager::handle_block_request(std::shared_ptr<peer::Peer> peer, uint32_t piece_index, uint32_t block_offset, uint32_t length) {
        std::cerr << "handle_block_request ptr is " << peer << "\n";
        active_torrent.read_block(piece_index, block_offset, length, [&, peer](std::shared_ptr<std::vector<uint8_t>> data) {
            std::cerr << "read_block_handler\n";
            peer->send_block(piece_index, block_offset, data);
        });
    }
};
