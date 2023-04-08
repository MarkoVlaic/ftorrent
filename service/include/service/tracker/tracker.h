#ifndef FTORRENT_TRACKER_H
#define FTORRENT_TRACKER_H

#include <array>
#include <vector>

#include "../sha1.h"
#include "../util.h"
#include "../types.h"
#include "../serialization.h"

namespace ftorrent {
namespace tracker {

    class Tracker {
    public:
        Tracker(const sha1::Hash& h, const ftorrent::types::PeerId& pid, uint16_t p):
        info_hash{h}, peer_id{pid}, listen_port{p} {
            std::vector<uint8_t> key_bytes = ftorrent::util::RandomByteGenerator::getInstance().generate(4);

            serialization::Deserializer key_deser{key_bytes};
            serialization::deserialize(key, key_deser);
        }

        virtual void run() = 0;
        virtual ~Tracker() = default;

    protected:
        const sha1::Hash info_hash;
        const ftorrent::types::PeerId peer_id;
        uint16_t listen_port;
        uint32_t key;

        uint64_t downloaded = 0;
        uint64_t uploaded = 0;
        uint64_t left = 12000;
    };

}; // tracker
}; // ftorrent

#endif
