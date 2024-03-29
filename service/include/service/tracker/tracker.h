#ifndef FTORRENT_TRACKER_H
#define FTORRENT_TRACKER_H

#include <array>
#include <vector>
#include <memory>
#include <functional>

#include "../sha1.h"
#include "../util.h"
#include "../types.h"
#include "../serialization.h"

namespace ftorrent {
namespace tracker {
    using PeerHandler = std::function<void(std::vector<types::PeerDescriptor>)>;

    class Tracker : public std::enable_shared_from_this<Tracker>{
    public:
        Tracker(const ftorrent::types::Hash& h, const ftorrent::types::PeerId& pid, uint16_t p, PeerHandler ph):
        info_hash{h}, peer_id{pid}, listen_port{p}, on_peers{ph} {
            std::vector<uint8_t> key_bytes = ftorrent::util::RandomByteGenerator::getInstance().generate(4);

            serialization::Deserializer key_deser{key_bytes};
            serialization::deserialize(key, key_deser);
        }

        virtual ~Tracker() = default;

        void start() {
            run();
        }

    protected:
        virtual void run() = 0;

        const types::Hash info_hash;
        const ftorrent::types::PeerId peer_id;
        uint16_t listen_port;
        uint32_t key;

        PeerHandler on_peers;

        uint64_t downloaded = 0;
        uint64_t uploaded = 0;
        uint64_t left = 12000;
    };

}; // tracker
}; // ftorrent

#endif
