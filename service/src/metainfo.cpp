#include <fstream>
#include <variant>
#include <iostream>

#include "service/metainfo.h"
#include "service/sha1.h"
#include "service/types.h"

namespace ftorrent {
    Metainfo::Metainfo(const std::string& path) {
            std::ifstream metainfo_file{path, std::ios::binary};
            auto decoded = bencode::decode(metainfo_file);

            auto metainfo = std::get<bencode::dict>(decoded);
            announce = std::get<bencode::string>(metainfo["announce"]);

            auto info = std::get<bencode::dict>(metainfo["info"]);
            piece_length = std::get<bencode::integer>(info["piece length"]);

            auto pieces_string = std::get<bencode::string>(info["pieces"]);
            uint32_t num_pieces = pieces_string.size() / 20;
            pieces.reserve(num_pieces);

            for(int i=0;i<num_pieces;i++) {
                types::Hash hash;
                pieces_string.copy((char*)hash.data(), 20, i*20);
                pieces.push_back(hash);
            }

            length = std::get<bencode::integer>(info["length"]);

            info_hash = sha1::computeHash(bencode::encode(info));

            std::cout << "info hash: " << std::hex;
            for(auto byte : info_hash) {
                std::cout << "\\x" << (int) byte;
            }

            std::cout << std::endl;
        }
};
