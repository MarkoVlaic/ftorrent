#include <boost/asio.hpp>
#include <boost/endian/conversion.hpp>
#include <vector>

#include "service/tracker/tracker.h"
#include "service/tracker/udp_tracker.h"
#include "service/serialization.h"
#include "service/types.h"
#include "service/random_byte_generator.h"

namespace ftorrent {
namespace serialization {
    template<>
    void serialize<tracker::udp::Request>(tracker::udp::Request req, Serializer& serializer) {
        serialize(boost::endian::native_to_big(req.connection_id), serializer);
        serialize(boost::endian::native_to_big(req.action), serializer);
        serialize(boost::endian::native_to_big(req.transaction_id), serializer);
    }

    template<>
    void serialize<tracker::udp::ConnectionRequest>(tracker::udp::ConnectionRequest req, Serializer& serializer) {
        serialize(static_cast<tracker::udp::Request>(req), serializer);
    }

    template<>
    void deserialize<tracker::udp::Response>(tracker::udp::Response& res, Deserializer& deserializer) {
        deserialize(res.action, deserializer);
        deserialize(res.transaction_id, deserializer);

        res.action = boost::endian::big_to_native(res.action);
        res.transaction_id = boost::endian::big_to_native(res.transaction_id);
    }

    template<>
    void deserialize<tracker::udp::ConnectionResponse>(tracker::udp::ConnectionResponse& res, Deserializer& deserializer) {
        deserialize(static_cast<tracker::udp::Response&>(res), deserializer);

        deserialize(res.connection_id, deserializer);
        res.connection_id = boost::endian::big_to_native(res.connection_id);
    }
};

namespace tracker {
    UdpTracker::UdpTracker(boost::asio::io_context& ioc, std::string hostname, std::string port, const sha1::Hash& h, const ftorrent::types::PeerId& pid):
    Tracker{h, pid}, io_context{ioc}, socket{ioc} {
        std::cout << "UdpTracker::UdpTracker()\n";
        socket.open(boost::asio::ip::udp::v4());

        boost::asio::ip::udp::resolver resolver{io_context};
        auto endpoints = resolver.resolve(boost::asio::ip::udp::v4(), hostname, port);
        if(endpoints.empty()) {
            throw ftorrent::Exception{"Cannot resolve tracker"};
        }
        server_endpoint = *endpoints.begin();

        udp::Request req = udp::ConnectionRequest{};
        req.action = 0x12345678;
        req.transaction_id = 0x87654321;

        /*auto serialized = serialization::serialize(req.connection_id);
        std::cout << "serialized: " << std::hex;
        for(auto byte: serialized) {
            std::cout << (int)byte << " ";
        }*/

        std::cout << "befire serialize\n";
        serialization::Serializer ser;
        serialization::serialize(req, ser);
        auto serialized = ser.data();
        std::cout << "serialized: " << std::hex;
        for(auto byte: serialized) {
            std::cout << (int)byte << " ";
        }
        std::cout << std::endl;
    }

    void UdpTracker::run() {
        udp::ConnectionRequest req;
        sendRequest(req);
    }

    template<typename RequestType>
    void UdpTracker::sendRequest(RequestType& req) {
        serialization::Deserializer tid_deser{ftorrent::util::RandomByteGenerator::getInstance().generate(4)};
        int32_t transaction_id;
        //deserialize(transaction_id, tid_deser);
        //req.transaction_id = transaction_id;
        req.transaction_id = 0x123;

        serialization::Serializer serializer;
        serialization::serialize(req, serializer);
        req_buf = serializer.data();

        /*socket.async_send_to(boost::asio::buffer(req_buf), server_endpoint, [this](boost::system::error_code, long unsigned int){
            std::cout << "req sent" << std::endl;
            this->getResponse();
        });*/

        socket.send_to(boost::asio::buffer(req_buf), server_endpoint);
        boost::asio::ip::udp::endpoint sender_endpoint;
        std::array<uint8_t, 1020> arr_buf;
        socket.receive_from(boost::asio::buffer(arr_buf), sender_endpoint);

        std::cout << "recieve done";
    }

    void UdpTracker::getResponse() {
        //res_buf = std::vector<uint8_t>{};
        res_buf.clear();
        std::cout << "open " << socket.is_open();
        //res_buf.resize(1020); /*TODO: Change magic to something more smart*/
        std::array<uint8_t, 1020> arr_buf;
        /*socket.async_receive_from(boost::asio::buffer(arr_buf), server_endpoint, [this](boost::system::error_code, long unsigned int) {
            std::cout << "response" << std::endl;
            /*serialization::Deserializer deser{this->res_buf};
            udp::Response res;
            deserialize(res, deser);

            std::cout << "recieved bytes: " << std::hex;
            for(auto byte : this->res_buf) {
                std::cout << (int) byte << " ";
            }

            std::cout << "Response { action: " << res.action << ", transaction_id: " << res.transaction_id << std::endl;
        });*/

        boost::asio::ip::udp::endpoint sender_endpoint;
        socket.receive_from(boost::asio::buffer(arr_buf), sender_endpoint);
        std::cout << "receive done" << std::endl;
    }
};

};
