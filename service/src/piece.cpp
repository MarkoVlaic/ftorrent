#include <cmath>

#include "service/piece.h"
#include "service/exception.h"

namespace ftorrent {
    Piece::Piece(uint32_t i, types::Hash h, uint32_t s): index{i}, hash{h}, size{s} {
        uint32_t num_blocks = ceil((size * 1.0) / nominal_block_size);
        blocks.reserve(num_blocks);

        for(int i=0;i<num_blocks;i++) {
            uint32_t bs;
            if(i == num_blocks - 1) {
                bs = size - i * nominal_block_size;
            } else {
                bs = nominal_block_size;
            }

            blocks.push_back(std::make_shared<Block>(bs, i * nominal_block_size));
        }
    }

    void Piece::block_written(const boost::system::error_code &e, uint32_t offset) {
        if(e.value() != 0) {
            throw ftorrent::Exception(e.message());
        }

        auto block = get_block_by_offset(offset);

        block->lock();
        block->set_complete(true);;
        std::vector<uint8_t> empty;
        block->write_buf.swap(empty); // empty the vector so it does not take memory
        block->unlock();

        complete_blocks++;
    }
};
