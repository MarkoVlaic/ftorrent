#ifndef FTORRENT_PEER_ERRORS_H
#define FTORRENT_PEER_ERRORS_H

#include <string>
#include <stdexcept>

namespace ftorrent {
namespace peer {
    struct HandshakeError : public std::runtime_error {
        HandshakeError(std::string m): runtime_error{m} {}
    };
};
};

#endif
