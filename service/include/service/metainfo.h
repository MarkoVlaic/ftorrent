#ifndef ZAVRSNI_METAINFO_H
#define ZAVRSNI_METAINFO_H

#include <string>
#include <vector>
#include <fstream>

#include "service/sha1.h"
#include "service/bencode.h"

namespace ftorrent {
    struct Metainfo {
        Metainfo(std::string path) {
            std::ifstream metainfo_file{path, std::ios::binary};
        }

        std::string announce;

        uint32_t piece_length;
        std::vector<sha1::Hash> pieces;

        uint32_t length;
    };
}; // ftorrent

#endif //ZAVRSNI_METAINFO_H
