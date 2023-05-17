#include <mutex>

#include "service/block.h"

namespace ftorrent {
    bool Block::try_lock() {
        return mut.try_lock();
    }

    void Block::lock() {
        mut.lock();
    }

    bool Block::requested() {
        bool locked = mut.try_lock_shared();
        if(!locked) {
            return true;
        }

        bool result = requested_;
        mut.unlock_shared();
        return result;
    }

    bool Block::complete() {
        bool locked = mut.try_lock_shared();
        if(!locked) {
            return true;
        }

        bool result = complete_;
        mut.unlock_shared();
        return result;
    }

    void Block::unlock() {
        mut.unlock();
    }

    void Block::set_requested(bool r) {
        requested_ = r;
    }

    void Block::set_complete(bool c) {
        complete_ = c;
    }

};
