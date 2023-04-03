#include <boost/bind.hpp>
#include <algorithm>
#include <random>
#include <unistd.h>
#include <chrono>

#include "service/manager.h"
#include "service/random_byte_generator.h"

namespace ftorrent {
    void Manager::run() {
        for(int i=0;i<num_threads;i++) {
            thread_pool.create_thread(
                boost::bind(&boost::asio::io_context::run, &io_context)
            );
        }

        thread_pool.join_all();
    }

    types::PeerId Manager::generatePeerId() {
        types::PeerId result;
        char start[] = "ftorrent";

        std::copy(start, start + 8, result.begin());
        uint32_t pid = getpid();
        for(int i=0;i<4;i++) {
            result[8 + i] = (uint8_t)((pid >> i*8) & 0xF);
        }

        util::RandomByteGenerator& generator = util::RandomByteGenerator::getInstance();
        generator.generate(result.begin() + 10, result.end());

        return result;
    }
};
