#ifndef ZAVRSNI_TORRENT_H
#define ZAVRSNI_TORRENT_H

#include <cstdint>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/random_access_file.hpp>

#include "service/metainfo.h"

namespace ftorrent {
namespace torrent {
    struct Block {
        Block(uint32_t s, uint32_t o): size{s}, offset{o} {}

        uint32_t size;
        uint32_t offset;
        bool complete = false;

        std::vector<uint8_t> write_buf;
    };

    struct Piece {
        Piece(sha1::Hash h): Piece{h, 0} {}
        Piece(sha1::Hash h, uint32_t s);

        sha1::Hash hash;
        uint32_t size;

        constexpr static uint32_t nominal_block_size = 1 << 14; // 16 kB
        std::vector<Block> blocks;

        std::vector<uint8_t> validation_buf;

        void blockWritten(const boost::system::error_code& e, uint32_t offset);

        bool complete() {
            return complete_blocks == blocks.size();
        }

        Block& getBlockByOffset(uint64_t offset) {
            uint64_t index = offset / nominal_block_size;
            return blocks[index];
        }

        void reset() {
            for(Block& block : blocks) {
                block.complete = false;
            }
            complete_blocks = 0;
        }

    private:
        uint32_t complete_blocks = 0;
    };

    class Torrent {
    public:
        Torrent(const ftorrent::Metainfo& metainfo, const std::string& output_path, boost::asio::io_context& io_context);
        void writeBlock(uint64_t piece_index, uint64_t block_offset, const std::vector<uint8_t>& data);
    private:
        uint64_t size;
        uint64_t nominal_piece_size;
        std::vector<Piece> pieces;

        boost::asio::random_access_file out_file;
        boost::asio::strand<boost::asio::io_context::executor_type> strand;
        std::vector<std::vector<uint8_t>> keep_alive;

        void validatePiece(uint64_t piece_index);
    };
} // torrent
}; // ftorrent

#endif //ZAVRSNI_TORRENT_H
