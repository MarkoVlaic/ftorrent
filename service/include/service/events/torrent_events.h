#ifndef FTORRENT_TORRENT_EVENTS_H
#define FTORRENT_TORRENT_EVENTS_H

#include "events.h"
#include "../torrent.h"

namespace ftorrent {
namespace events {
    struct PieceVerifiedEvent : Event {
        PieceVerifiedEvent(const ftorrent::torrent::Piece& p): piece{p} {
            group = EventGroup::torrent();
            name = "PieceVerified";
        }

        const torrent::Piece& piece;
    };
}; // events
}; // ftorrent

#endif
