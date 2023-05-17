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
#include "service/piece.h"
#include "service/types.h"

namespace ftorrent {
    class Torrent {
    public:
        Torrent(
            const ftorrent::Metainfo& metainfo, const std::string& output_path, boost::asio::io_context& io_context,
            const std::vector<std::shared_ptr<Piece>>& pcs, std::function<void(uint32_t)> piece_complete_handler
        );

        void write_block(uint64_t piece_index, uint64_t block_offset, const std::vector<uint8_t>& data);
        void read_block(uint32_t piece_index, uint32_t block_offset, uint32_t length, std::function<void(std::shared_ptr<std::vector<uint8_t>>)> handler);

    private:
        void validate_piece(uint64_t piece_index);
    private:
        uint64_t size;
        uint64_t nominal_piece_size;
        std::vector<std::shared_ptr<Piece>> pieces;

        uint64_t downloaded = 0;
        uint64_t uploaded = 0;
        uint32_t validated = 0;

        boost::asio::random_access_file out_file;
        boost::asio::strand<boost::asio::io_context::executor_type> strand;

        std::vector<std::shared_ptr<std::vector<uint8_t>>> read_buffers;
        std::mutex read_mutex;

        std::function<void(uint32_t)> piece_complete;
    };
}; // ftorrent

#endif //ZAVRSNI_TORRENT_H
