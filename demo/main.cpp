#include <array>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <memory>
#include <vector>

#include "service/manager.h"
#include "service/peer/peer_connection.h"
#include "service/metainfo.h"
#include "service/types.h"
#include "service/serialization.h"
#include "service/util.h"

int main(int argc, char* argv[]) {
    //ftorrent::Manager manager{"./test-file.torrent", "demo-out.png", 16};
    //manager.run();

    boost::asio::io_context io_context;
    boost::thread_group thread_pool;

    /*for(int i=0;i<16;i++) {
        thread_pool.create_thread(
            boost::bind(&boost::asio::io_context::run, &io_context)
        );
    }*/

    std::cout << "argc " << argc << "\n";

    std::vector<bool> bitfield = {
        true, false, true, true, false, false, true, false,
        true, true, false, false, true, false, true, true,
        false, false, true, false, true
    };

    boost::asio::ip::tcp::resolver resolver{io_context};
    auto endpoints = resolver.resolve(argv[1], argv[2]);

    ftorrent::Metainfo metainfo{"./test-file.torrent"};
    ftorrent::types::PeerId pid = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    std::cout << "should call ctor\n";
    ftorrent::peer::PeerConnection peer_connection{io_context, endpoints, metainfo.info_hash, pid};

    std::vector<uint8_t> mock_block(1024, 1);

    std::shared_ptr<ftorrent::peer::messages::Message> messages[] = {
        std::make_shared<ftorrent::peer::messages::Message>(0, ftorrent::peer::messages::EMessageId::KEEP_ALIVE),
        std::make_shared<ftorrent::peer::messages::Choke>(),
        std::make_shared<ftorrent::peer::messages::Unchoke>(),
        std::make_shared<ftorrent::peer::messages::Interested>(),
        std::make_shared<ftorrent::peer::messages::NotInterested>(),
        std::make_shared<ftorrent::peer::messages::Have>(12),
        std::make_shared<ftorrent::peer::messages::BitField>(bitfield),
        std::make_shared<ftorrent::peer::messages::Request>(1, 1024, 1024),
        std::make_shared<ftorrent::peer::messages::Piece>(1, 1024, mock_block),
        std::make_shared<ftorrent::peer::messages::Cancel>(1, 1024, 1024),
    };


    for(auto msg : messages) {
        peer_connection.send(msg);
    }

    // thread_pool.join_all();
    io_context.run();

}
