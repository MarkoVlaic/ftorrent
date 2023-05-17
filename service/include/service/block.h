#ifndef FTORRENT_BLOCK_H
#define FTORRENT_BLOCK_H

#include <cstdint>
#include <vector>
#include <shared_mutex>

namespace ftorrent {
    struct Block {
        Block(uint32_t s, uint32_t o): size{s}, offset{o} {}

        const uint32_t size;
        const uint32_t offset;

        std::vector<uint8_t> write_buf;

        bool try_lock();
        void lock();
        void unlock();

        bool requested();
        bool complete();

        void set_requested(bool);
        void set_complete(bool);

    private:
        bool complete_ = false;
        bool requested_ = false;
        std::shared_mutex mut;
    };
};

#endif
