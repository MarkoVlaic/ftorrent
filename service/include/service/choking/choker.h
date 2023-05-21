#ifndef FTORRENT_CHOKING_CHOKER_H
#define FTORRENT_CHOKING_CHOKER_H

#include "../peer/peer.h"

namespace ftorrent {
namespace choking {
    struct Choker {
        Choker() = default;
        virtual ~Choker() = default;
        virtual void start() = 0;
        virtual void add_peer(std::shared_ptr<peer::AbstractPeer> p) = 0;
        virtual void remove_peer(std::shared_ptr<peer::AbstractPeer> p) = 0;
        virtual void stop() = 0;
    };
};
};

#endif
