#ifndef FTORRENT_TRACKER_H
#define FTORRENT_TRACKER_H

#include <array>

#include "../sha1.h"
#include "../random_byte_generator.h"
#include "../types.h"

namespace ftorrent {
namespace tracker {

    class Tracker {
    public:
        Tracker(const sha1::Hash& h, const ftorrent::types::PeerId& pid):
        info_hash{h}, peer_id{pid} {
            ftorrent::util::RandomByteGenerator::getInstance().generate(key.begin(), key.end());
        }

        virtual void run() = 0;
        virtual ~Tracker() = default;

    protected:
        const sha1::Hash info_hash;
        const ftorrent::types::PeerId peer_id;

        std::array<uint8_t, 4> key;
    };

}; // tracker
}; // ftorrent

#endif
