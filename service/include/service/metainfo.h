#ifndef ZAVRSNI_METAINFO_H
#define ZAVRSNI_METAINFO_H

#include <string>
#include <vector>

#include "service/sha1.h"
#include "service/bencode.h"

namespace ftorrent {
    struct Metainfo {
        explicit Metainfo(const std::string& path);

        std::string announce;
        uint64_t piece_length;
        std::vector<sha1::Hash> pieces;
        uint64_t length;
        sha1::Hash info_hash;
    };
}; // ftorrent

#endif //ZAVRSNI_METAINFO_H
