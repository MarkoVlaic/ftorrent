#ifndef FTORRENT_UDP_TRACKER_H
#define FTORRENT_UDP_TRACKER_H

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <vector>
#include <array>
#include <memory>
#include <chrono>

#include "tracker.h"
#include "../exception.h"
#include "../types.h"

#define BOOST_ASIO_ENABLE_HANDLER_TRACKING 1

namespace ftorrent {
namespace tracker {
namespace udp {
    enum Action {

    };

    struct Request : public ftorrent::serialization::Serializable {
        Request() {
            std::vector<uint8_t> tid_bytes = ftorrent::util::RandomByteGenerator::getInstance().generate(4);
            serialization::Deserializer tid_deser{tid_bytes};
            deserialize(transaction_id, tid_deser);
        }
        virtual ~Request() = default;

        virtual void serialize(ftorrent::serialization::Serializer&) override;

        uint64_t connection_id;
        uint32_t action;
        uint32_t transaction_id;
    };

    struct ConnectRequest : public Request {
        ConnectRequest() {
            connection_id = 0x41727101980;
            action = 0;
        }
        ~ConnectRequest() = default;

        virtual void serialize(ftorrent::serialization::Serializer& s) override {
            std::cout << "serialize conn\n";
            Request::serialize(s);
        };
    };

    struct AnnounceRequest : public Request {
        AnnounceRequest() {
            action = 1;
        }
        ~AnnounceRequest() = default;

        virtual void serialize(ftorrent::serialization::Serializer&) override;

        enum Event {
            NONE = 0,
            COMPLETED,
            STARTED,
            STOPPED
        };

        ftorrent::types::Hash info_hash;
        ftorrent::types::PeerId peer_id;
        uint64_t downloaded;
        uint64_t left;
        uint64_t uploaded;
        Event event;
        uint32_t ip_addr;
        uint32_t key;
        uint32_t num_want;
        uint16_t port;
    };

    struct Response {
        uint32_t action;
        uint32_t transaction_id;
    };

    struct ConnectionResponse : public Response {
      uint64_t connection_id;
    };

    struct AnnounceResponse : public Response {
        uint32_t interval;
        uint32_t leechers;
        uint32_t seeders;
        std::vector<types::PeerDescriptor> peers;
    };

    struct ErrorResponse : public Response {
        std::string message;
    };

    class RetryTimer {
    public:
        RetryTimer(boost::asio::strand<boost::asio::io_context::executor_type>& strand):
            timer{strand, boost::asio::steady_timer::time_point::max()} {}

        void start(std::function<void()> handler);
        void stop();

    private:
        boost::asio::steady_timer timer;
        uint8_t retransmission_index = 0;
        bool stopped = true;

        void waitHandler(const boost::system::error_code& e, std::function<void()> handler);

        void increment() {
            retransmission_index = std::min(retransmission_index + 1, 8);
        }

        std::chrono::seconds getExpire() {
            uint32_t seconds = 15 + (1u << retransmission_index);
            return std::chrono::seconds(seconds);
        }
    };

    class RequestQueue {
    public:
        void insert(std::shared_ptr<ConnectRequest> request) {
            queue[1] = request;
        }

        void insert(std::shared_ptr<AnnounceRequest> request) {
            queue[0] = request;
        }

        std::shared_ptr<Request> top() const {
            if(queue[1] != nullptr)
                return queue[1];
            if(queue[0] != nullptr) {
                return queue[0];
            }

            throw ftorrent::Exception{"top on empty queue"};
        }

        bool empty() const {
            return queue[0] == nullptr && queue[1] == nullptr;
        }

        void remove(std::shared_ptr<Request> req) {
            for(auto& element : queue) {
                if(element == req) {
                    std::cout << "remove";
                    element = nullptr;
                }
            }
        }

    private:
        std::array<std::shared_ptr<Request>, 2> queue = {nullptr, nullptr};
    };
}; // udp

    class UdpTracker : public Tracker {
    public:
        UdpTracker(boost::asio::io_context& ioc, std::string hostname, std::string port, const types::Hash& h, const ftorrent::types::PeerId& pid, uint16_t listen_port, PeerHandler ph);

        ~UdpTracker() = default;

    protected:
        void run() override;

    private:
        void sendRequest();
        std::shared_ptr<udp::ConnectRequest> makeConnectRequest();
        std::shared_ptr<udp::AnnounceRequest> makeAnnounceRequest();

        void getResponse();
        void handleResponse();

        template<typename ReqType>
        void scheduleRequest(boost::asio::steady_timer& timer, std::function<std::shared_ptr<ReqType>()> req_factory, uint32_t seconds);

    private:
        boost::asio::strand<boost::asio::io_context::executor_type> strand;
        boost::asio::ip::udp::socket socket;
        boost::asio::ip::udp::endpoint server_endpoint;

        udp::RetryTimer retry_timer;
        boost::asio::steady_timer announce_timer;
        boost::asio::steady_timer connect_timer;

        std::vector<uint8_t> req_buf;
        std::vector<uint8_t> res_buf;

        udp::RequestQueue req_queue;
        std::shared_ptr<udp::Request> cur_req = nullptr;
        bool req_sent = false;

        static constexpr uint64_t MAGIC_CONNECTION_ID = 0x41727101980;

        uint64_t connection_id = MAGIC_CONNECTION_ID;
        uint32_t announce_interval;
        udp::AnnounceRequest::Event cur_announce_event = udp::AnnounceRequest::Event::STARTED;
    };
}; // tracker
}; // ftorrent

#endif
