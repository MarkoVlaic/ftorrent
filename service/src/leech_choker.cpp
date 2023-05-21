#include <algorithm>
#include <random>
#include <chrono>
#include <functional>

#include "service/choking/leech_choker.h"

namespace ftorrent {
namespace choking {
    LeechChoker::LeechChoker(boost::asio::io_context& ioc): timer{boost::asio::make_strand(ioc), boost::asio::steady_timer::time_point::max()} {}

    void LeechChoker::start() {
        timer.expires_after(std::chrono::seconds(10));
        timer.async_wait([this](auto e){
            this->timer_handler(e);
        });

    }

    void LeechChoker::add_peer(std::shared_ptr<peer::AbstractPeer> peer) {
        peers.push_back(peer);
    }

    void LeechChoker::remove_peer(std::shared_ptr<peer::AbstractPeer> peer) {
        auto it = std::find(peers.begin(), peers.end(), peer);
        if(it == peers.end())
            return;

        peers.erase(it);
    }

    void LeechChoker::select() {
        iteration = (iteration + 1) % 3;
        std::sort(peers.begin(), peers.end(), DownloadRateCmp{});
        std::vector<std::shared_ptr<peer::AbstractPeer>> interested;
        std::copy_if(peers.begin(), peers.end(), std::back_inserter(interested), [](auto peer){
            return peer->get_upload_interested();
        });
        std::vector<std::shared_ptr<peer::AbstractPeer>> result;

        std::shared_ptr<peer::AbstractPeer> optimistic_unchoke = nullptr;
        if(iteration == 0) {
            std::sample(interested.begin(), interested.end(), std::back_inserter(result), 1, std::mt19937{std::random_device{}()});
        }

        int to_add = active_size - result.size();
        for(int i=0;i<to_add && i < interested.size();i++) {
            result.push_back(interested[i]);
        }

        for(auto peer : active) {
            auto it = std::find(result.begin(), result.end(), peer);
            if(it != active.end()) {
                peer->choke_upload();
            }
        }

        active = result;
        for(auto peer : active) {
            peer->unchoke_upload();
        }
    }

    void LeechChoker::timer_handler(const boost::system::error_code& e)
    {
        if(e)
            return;

        select();
        timer.expires_after(std::chrono::seconds(10));
        timer.async_wait([this](auto e){
            this->timer_handler(e);
        });
    }

    void LeechChoker::stop() {
        timer.cancel();
    }

}; // choking
}; // ftorrent
