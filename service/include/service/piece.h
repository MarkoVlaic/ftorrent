#ifndef FTORRENT_PIECE_H
#define FTORRENT_PIECE_H

#include <boost/system.hpp>
#include <vector>
#include <unordered_map>
#include <memory>

#include "service/types.h"
#include "service/block.h"

namespace ftorrent {
    struct Piece {
        Piece(uint32_t i, types::Hash h): Piece{i, h, 0} {}
        Piece(uint32_t i, types::Hash h, uint32_t s);

        uint32_t index;
        types::Hash hash;
        uint32_t size;

        constexpr static uint32_t nominal_block_size = 1 << 14; // 16 kB

        std::vector<uint8_t> validation_buf;
        std::unordered_map<uint32_t, std::vector<uint8_t>> block_read_buffers;
        std::vector<std::shared_ptr<Block>> blocks;

        void block_written(const boost::system::error_code& e, uint32_t offset);

        bool complete() {
            return complete_blocks == blocks.size();
        }

        std::shared_ptr<Block> get_block_by_offset(uint64_t offset) {
            uint64_t index = offset / nominal_block_size;
            return blocks[index];
        }

        void reset() {
            for(auto block : blocks) {
                block->lock();
                block->set_complete(false);
                block->unlock();
            }
            complete_blocks = 0;
        }

        uint32_t get_availability() const {
            return availability;
        }

        void set_availability(uint32_t a) {
            availability = a;
        }
    private:
        uint32_t complete_blocks = 0;
        uint32_t availability = 0;
    };
};

#endif
