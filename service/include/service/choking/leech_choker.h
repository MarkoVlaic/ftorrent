#ifndef FTORRENT_LEECH_CHOKER_H
#define FTORRENT_LEECH_CHOKER_H

#include <vector>
#include <memory>
#include <boost/asio.hpp>

#include "./choker.h"
#include "../peer/abstract_peer.h"

namespace ftorrent {
namespace choking {
    struct DownloadRateCmp {
        bool operator()(const std::shared_ptr<peer::AbstractPeer>& lhs, const std::shared_ptr<peer::AbstractPeer>& rhs) const {
            return lhs->get_download_rate() < rhs->get_download_rate();
        }
    };

    class LeechChoker : public Choker {
    public:
        LeechChoker(boost::asio::io_context& ioc);
        void start() override;
        void add_peer(std::shared_ptr<peer::AbstractPeer> p) override;
        void remove_peer(std::shared_ptr<peer::AbstractPeer> p) override;
        void stop() override;
    private:
        void select();
        void timer_handler(const boost::system::error_code&);
    private:
        std::vector<std::shared_ptr<peer::AbstractPeer>> peers;
        std::vector<std::shared_ptr<peer::AbstractPeer>> active;
        uint8_t iteration = 0;
        boost::asio::steady_timer timer;


        // TODO pull this from config
        constexpr static uint32_t active_size = 4;
    };
};
};

#endif
