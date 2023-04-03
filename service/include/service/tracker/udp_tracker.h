#ifndef FTORRENT_UDP_TRACKER_H
#define FTORRENT_UDP_TRACKER_H

#include <boost/asio.hpp>
#include <iostream>
#include <vector>

#include "tracker.h"
#include "../exception.h"

namespace ftorrent {
namespace tracker {


    class UdpTracker : public Tracker {
    public:
        UdpTracker(boost::asio::io_context& ioc, std::string hostname, std::string port, const sha1::Hash& h, const ftorrent::types::PeerId& pid);

        void run() override;
        ~UdpTracker() = default;

    private:
        boost::asio::io_context& io_context;
        boost::asio::ip::udp::socket socket;
        boost::asio::ip::udp::endpoint server_endpoint;

        std::vector<uint8_t> req_buf;
        std::vector<uint8_t> res_buf;

        template<typename RequestType>
        void sendRequest(RequestType&);

        void getResponse();
    };

namespace udp {
    struct Request {
        int64_t connection_id;
        int32_t action;
        int32_t transaction_id;
    };

    struct ConnectionRequest: public Request {
        ConnectionRequest() {
            connection_id = 0x41727101980;
            action = 0;
        }
    };

    struct Response {
        int32_t action;
        int32_t transaction_id;
    };

    struct ConnectionResponse : public Response {
      int64_t connection_id;
    };
}; // udp
}; // tracker
}; // ftorrent

#endif
