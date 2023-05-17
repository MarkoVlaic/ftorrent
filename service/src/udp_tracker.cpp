#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/endian/conversion.hpp>
#include <vector>

#include "service/tracker/tracker.h"
#include "service/tracker/udp_tracker.h"
#include "service/serialization.h"
#include "service/types.h"
#include "service/util.h"

namespace boost {
    namespace endian {
        template<>
        std::array<uint32_t, 2> native_to_big(std::array<uint32_t, 2> arr) noexcept {
            return { native_to_big(arr[0]), native_to_big(arr[1]) };
        }

        template<>
        std::array<uint32_t, 2> big_to_native(std::array<uint32_t, 2> arr) noexcept {
            return { big_to_native(arr[0]), big_to_native(arr[1]) };
        }
    }; // endian
}; // boost

namespace ftorrent {
namespace serialization {
    template<>
    void deserialize(tracker::udp::Response& res, Deserializer& deserializer) {
        deserialize(res.action, deserializer);
        deserialize(res.transaction_id, deserializer);

        res.action = boost::endian::big_to_native(res.action);
        res.transaction_id = boost::endian::big_to_native(res.transaction_id);
    }

    template<>
    void deserialize(tracker::udp::ConnectionResponse& res, Deserializer& deserializer) {
        deserialize(static_cast<tracker::udp::Response&>(res), deserializer);

        deserialize(res.connection_id, deserializer);
        res.connection_id = boost::endian::big_to_native(res.connection_id);
    }

    template<>
    void deserialize(tracker::udp::AnnounceResponse& res, Deserializer& deserializer) {
        deserialize(static_cast<tracker::udp::Response&>(res), deserializer);

        deserialize(res.interval, deserializer);
        deserialize(res.leechers, deserializer);
        deserialize(res.seeders, deserializer);

        res.interval = boost::endian::big_to_native(res.interval);
        res.leechers = boost::endian::big_to_native(res.leechers);
        res.seeders = boost::endian::big_to_native(res.seeders);

        res.peers.reserve(res.leechers + res.seeders);
        for(int i=0;i<res.leechers + res.seeders;i++) {
            uint32_t ip;
            uint16_t port;

            deserialize(ip, deserializer);
            deserialize(port, deserializer);

            ip = boost::endian::big_to_native(ip);
            port = boost::endian::big_to_native(port);

            res.peers.emplace_back(ip, port);
        }
    }

    template<>
    void deserialize(tracker::udp::ErrorResponse& res, Deserializer& deserializer) {
        deserialize(static_cast<tracker::udp::Response&>(res), deserializer);
        deserialize(res.message, deserializer);
    }

}; // serialization

namespace tracker {
    namespace udp {
        void Request::serialize(ftorrent::serialization::Serializer& serializer) {
            ftorrent::serialization::serialize(boost::endian::native_to_big(connection_id), serializer);
            ftorrent::serialization::serialize(boost::endian::native_to_big(action), serializer);
            ftorrent::serialization::serialize(boost::endian::native_to_big(transaction_id), serializer);
        }

        void AnnounceRequest::serialize(ftorrent::serialization::Serializer& serializer) {
            Request::serialize(serializer);
            ftorrent::serialization::serialize(info_hash, serializer);
            ftorrent::serialization::serialize(peer_id, serializer);
            ftorrent::serialization::serialize(boost::endian::native_to_big(downloaded), serializer);
            ftorrent::serialization::serialize(boost::endian::native_to_big(left), serializer);
            ftorrent::serialization::serialize(boost::endian::native_to_big(uploaded), serializer);
            ftorrent::serialization::serialize(boost::endian::native_to_big(static_cast<uint32_t>(event)), serializer);
            ftorrent::serialization::serialize(boost::endian::native_to_big(ip_addr), serializer);
            ftorrent::serialization::serialize(boost::endian::native_to_big(key), serializer);
            ftorrent::serialization::serialize(boost::endian::native_to_big(num_want), serializer);
            ftorrent::serialization::serialize(boost::endian::native_to_big(port), serializer);
        }

        void RetryTimer::start(std::function<void()> handler) {
            stopped = false;
            timer.expires_after(getExpire());
            timer.async_wait(boost::bind(&RetryTimer::waitHandler, this, boost::asio::placeholders::error, handler));
        }

        void RetryTimer::stop() {
            timer.cancel();
            stopped = true;
            retransmission_index = 0;
        }

        void RetryTimer::waitHandler(const boost::system::error_code& e, std::function<void()> handler) {
            if(e) {
                if(e == boost::asio::error::operation_aborted) {
                    return;
                }
                return;
            }

            if(stopped) {
                return;
            }

            handler();
            increment();

            timer.expires_after(getExpire());
            timer.async_wait(boost::bind(&RetryTimer::waitHandler, this, boost::asio::placeholders::error, handler));
        }
    }; // udp

    UdpTracker::UdpTracker(
        boost::asio::io_context& ioc, std::string hostname, std::string port,
        const ftorrent::types::Hash& h, const ftorrent::types::PeerId& pid, uint16_t listen_port, PeerHandler ph
    ):
        Tracker{h, pid, listen_port, ph}, strand{boost::asio::make_strand(ioc)},
        socket{ioc}, retry_timer{strand},
        announce_timer{strand}, connect_timer{strand}
    {
        socket.open(boost::asio::ip::udp::v4());

        boost::asio::ip::udp::resolver resolver{ioc};
        auto endpoints = resolver.resolve(boost::asio::ip::udp::v4(), hostname, port);
        if(endpoints.empty()) {
            throw ftorrent::Exception{"Cannot resolve tracker"};
        }
        server_endpoint = *endpoints.begin();


    }

    void UdpTracker::run() {
        auto conn_req = makeConnectRequest();
        req_queue.insert(conn_req);

        auto announce_req = makeAnnounceRequest();
        req_queue.insert(announce_req);

        sendRequest();
    }

    void UdpTracker::sendRequest() {
        cur_req = req_queue.top();
        cur_req->connection_id = connection_id;

        serialization::Serializer serializer;
        serialization::serialize(cur_req, serializer);
        req_buf = serializer.data();

        socket.async_send_to(
            boost::asio::buffer(req_buf), server_endpoint,
            boost::asio::bind_executor(strand, [this](boost::system::error_code, long unsigned int){

            if(!this->req_sent) {
                this->getResponse();
                this->req_sent = true;
                retry_timer.start(boost::bind(&UdpTracker::sendRequest, this));
            }
        }));
    }

    std::shared_ptr<udp::ConnectRequest> UdpTracker::makeConnectRequest() {
        connection_id = MAGIC_CONNECTION_ID;
        return std::make_shared<udp::ConnectRequest>();
    }

    std::shared_ptr<udp::AnnounceRequest> UdpTracker::makeAnnounceRequest() {
        auto announce_req = std::make_shared<udp::AnnounceRequest>();
        announce_req->action = 1;
        announce_req->info_hash = info_hash;
        announce_req->peer_id = peer_id;
        announce_req->downloaded = downloaded;
        announce_req->left = left;
        announce_req->uploaded = uploaded;
        announce_req->event = cur_announce_event;
        announce_req->ip_addr = 0;
        announce_req->key = key;
        announce_req->num_want = 50;
        announce_req->port = listen_port;

        return announce_req;
    }

    void UdpTracker::getResponse() {
        // res_buf = std::vector<uint8_t>{};
        res_buf.clear();
        res_buf.resize(1020); /*TODO: Change magic to something more smart*/
        std::array<uint8_t, 1020> arr_buf;
        boost::asio::ip::udp::endpoint sender_endpoint;

        socket.async_receive_from(boost::asio::buffer(res_buf), sender_endpoint, [this](boost::system::error_code, long unsigned int) {
            retry_timer.stop();
            this->handleResponse();

            req_sent = false;
            cur_req = nullptr;

            if(!req_queue.empty()) {
                sendRequest();
            }
        });
    }

    void UdpTracker::handleResponse() {
        serialization::Deserializer deserializer{res_buf};
        udp::Response response_header;
        deserialize(response_header, deserializer);
        deserializer.reset();

        if(cur_req->transaction_id != response_header.transaction_id) {
            std::cout << "tid mismatch\n";
            return;
        }

        if(response_header.action == 0) {
            if(res_buf.size() < 16) {
                std::cerr << "Malformed tracker response" << std::endl;
                return;
            }

            req_queue.remove(cur_req);

            udp::ConnectionResponse response;
            deserialize(response, deserializer);

            connection_id = response.connection_id;

            scheduleRequest<udp::ConnectRequest>(
                connect_timer,
                std::bind(&UdpTracker::makeConnectRequest, this),
                60
            );

            return;
        }

        if(response_header.action == 1) {
            if(res_buf.size() < 20) {
                std::cerr << "Malformed tracker response" << std::endl;
                return;
            }

            req_queue.remove(cur_req);

            udp::AnnounceResponse response;
            deserialize(response, deserializer);

            announce_interval = response.interval;

            std::cerr << "peers:\n";

            for(auto pd : response.peers) {
                std::cout << "peer: " << pd.ip << " " << pd.port << std::endl;
            }
            on_peers(response.peers);

            scheduleRequest<udp::AnnounceRequest>(
                announce_timer,
                std::bind(&UdpTracker::makeAnnounceRequest, this),
                announce_interval
            );
            return;
        }

        if(response_header.action == 3) {
            udp::ErrorResponse response;
            deserialize(response, deserializer);

            std::cerr << "tracker error: " << response.message;
            req_queue.remove(cur_req);

            scheduleRequest<udp::AnnounceRequest>(
                announce_timer,
                std::bind(&UdpTracker::makeAnnounceRequest, this),
                announce_interval
            );

            scheduleRequest<udp::ConnectRequest>(
                connect_timer,
                std::bind(&UdpTracker::makeConnectRequest, this),
                60
            );
        }
    }

    template<typename ReqType>
    void UdpTracker::scheduleRequest(boost::asio::steady_timer& timer, std::function<std::shared_ptr<ReqType>()> req_factory, uint32_t seconds) {
        timer.expires_after(std::chrono::seconds(seconds));
        timer.async_wait([this, req_factory](auto e){
            if(e) {
                return;
            }

            this->req_queue.insert(req_factory());
            if(!this->req_sent) {
                sendRequest();
            }
        });
    }
}; // tracker

}; // ftorrent
