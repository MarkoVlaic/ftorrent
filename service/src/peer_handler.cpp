#include <boost/asio.hpp>
#include <boost/endian.hpp>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <iostream> // TODO remove
#include <memory>

#include "service/peer/peer_handler.h"
#include "service/peer/handler_types.h"
#include "service/types.h"

namespace ftorrent {
namespace peer {
    using boost::asio::ip::tcp;
    PeerHandler::PeerHandler(
        boost::asio::io_context& ioc, types::Hash h, types::PeerId pid,
        uint64_t np, uint16_t lp,
        BlockRecievedHandler blk_rcvd, BlockRequestHandler blk_req
    ):
        io_context{ioc}, info_hash{h}, peer_id{pid}, num_pieces{np}, listen_port{lp},
        acceptor{io_context, tcp::endpoint(tcp::v4(), listen_port)}, block_recieved{blk_rcvd}, block_requested{blk_req}
    {
        // TODO: implement accept connection request
        struct ifaddrs* ifaddr;

        if(getifaddrs(&ifaddr) == -1) {
            // TODO signal critical fail
            std::cerr << "getifaddrsfailed\n";
        }

        char host[NI_MAXHOST];

        for(struct ifaddrs* ifa = ifaddr;ifa != NULL;ifa = ifa->ifa_next) {
            if(ifa->ifa_addr == NULL)
                continue;

            int family = ifa->ifa_addr->sa_family;

            if(family == AF_INET || family == AF_INET6) {
                int addr_size = (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6);
                int s = getnameinfo(ifa->ifa_addr, addr_size, host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
                if (s != 0) {
                    printf("getnameinfo() failed: %s\n", gai_strerror(s));
                    exit(EXIT_FAILURE);
                }

                std::string ip{host};
                local_endpoints.emplace_back(boost::asio::ip::make_address(host), listen_port);
            }
        }

        std::cerr << "local endpoints:\n";
        for(auto ep : local_endpoints) {
            std::cerr << ep << "\n";
        }
    }

    void PeerHandler::add_peer(types::PeerDescriptor descriptor) {
        char ip_str_buf[INET_ADDRSTRLEN];
        struct in_addr num_ip = {boost::endian::native_to_big(descriptor.ip)};
        if(inet_ntop(AF_INET, &num_ip, ip_str_buf, INET_ADDRSTRLEN) == NULL) {
            std::cerr << "INET NTOP err";
        };
        std::string ip_str{ip_str_buf};
        std::cerr << "resolve ip:port " << ip_str << " " << descriptor.port << "\n";

        tcp::resolver resolver{io_context};
        tcp::resolver::results_type endpoints = resolver.resolve(ip_str, std::to_string(descriptor.port), tcp::resolver::flags::numeric_service|tcp::resolver::flags::numeric_host);

        for(auto it = endpoints.begin();it != endpoints.end();it++) {
            if(std::find(local_endpoints.begin(), local_endpoints.end(), *it) != std::end(local_endpoints)) {
                std::cerr << "local endpoint found\n";
                return;
            }
        }

        auto peer = std::make_shared<Peer>(io_context, endpoints, info_hash, peer_id, num_pieces, std::bind(&PeerHandler::remove_peer, this, std::placeholders::_1), block_recieved, block_requested);
        peers.push_back(peer);
    }

    void PeerHandler::remove_peer(std::shared_ptr<Peer> peer) {
        std::cerr << "enter remove peer" << peer << "\nall peers:\n";
        for(auto p : peers) {
            std::cerr << p << " ";
        }
        std::cerr << "\n";

        auto it = std::find(peers.begin(), peers.end(), peer);
        if(it == std::end(peers)) {
            // TODO error possibly
            return;
        }
        std::cout << "remove peer\n";
        peers.erase(it);
    }

    void PeerHandler::piece_complete(uint32_t piece_index) {
        for(auto peer : peers) {
            peer->send_have(piece_index);
        }
    }
}; // peer
}; // ftorrent
