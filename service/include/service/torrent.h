#ifndef ZAVRSNI_TORRENT_H
#define ZAVRSNI_TORRENT_H

#include <cstdint>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/asio/random_access_file.hpp>
#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>

#include "service/metainfo.h"
#include "service/events/events.h"
#include "service/types.h"

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
        Piece(types::Hash h): Piece{h, 0} {}
        Piece(types::Hash h, uint32_t s);

        types::Hash hash;
        uint32_t size;

        constexpr static uint32_t nominal_block_size = 1 << 14; // 16 kB
        std::vector<Block> blocks;

        std::vector<uint8_t> validation_buf;
        std::unordered_map<uint32_t, std::vector<uint8_t>> block_read_buffers;

        void block_written(const boost::system::error_code& e, uint32_t offset);

        bool complete() {
            return complete_blocks == blocks.size();
        }

        Block& get_block_by_offset(uint64_t offset) {
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
        Torrent(
            const ftorrent::Metainfo& metainfo, const std::string& output_path, boost::asio::io_context& io_context,
            std::function<void(uint32_t)> piece_complete_handler
        );

        void write_block(uint64_t piece_index, uint64_t block_offset, const std::vector<uint8_t>& data);
        void read_block(uint32_t piece_index, uint32_t block_offset, uint32_t length, std::function<void(std::shared_ptr<std::vector<uint8_t>>)> handler);

    private:
        void validate_piece(uint64_t piece_index);
    private:
        uint64_t size;
        uint64_t nominal_piece_size;
        std::vector<Piece> pieces;

        uint64_t downloaded = 0;
        uint64_t uploaded = 0;

        boost::asio::random_access_file out_file;
        boost::asio::strand<boost::asio::io_context::executor_type> strand;

        std::vector<std::shared_ptr<std::vector<uint8_t>>> read_buffers;
        std::mutex read_mutex;

        std::function<void(uint32_t)> piece_complete;
    };
} // torrent
}; // ftorrent

#endif //ZAVRSNI_TORRENT_H
