#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>

#include "service/torrent.h"
#include "service/exception.h"
#include "service/events/torrent_events.h"
#include "service/types.h"

namespace ftorrent {
namespace torrent {
    Piece::Piece(types::Hash h, uint32_t s): hash{h}, size{s} {
        uint32_t num_blocks = ceil((size * 1.0) / nominal_block_size);
        blocks.reserve(num_blocks);

        for(int i=0;i<num_blocks;i++) {
            uint32_t bs;
            if(i == num_blocks - 1) {
                bs = size - i * nominal_block_size;
            } else {
                bs = nominal_block_size;
            }

            blocks.emplace_back(bs, i * nominal_block_size);
        }
    }

    void Piece::blockWritten(const boost::system::error_code &e, uint32_t offset) {
        if(e.value() != 0) {
            throw ftorrent::Exception(e.message());
        }

        Block& block = getBlockByOffset(offset);
        block.complete = true;
        std::vector<uint8_t> empty;
        block.write_buf.swap(empty); // empty the vector so it does not take memory
        complete_blocks++;
    }

    Torrent::Torrent(const ftorrent::Metainfo& metainfo, const std::string& output_path, boost::asio::io_context& io_context):
            size{metainfo.length}, nominal_piece_size{metainfo.piece_length},
            out_file{io_context, output_path, boost::asio::random_access_file::create | boost::asio::random_access_file::read_write},
            strand{boost::asio::make_strand(io_context)}
        {
            pieces.reserve(metainfo.pieces.size());
            for(auto it=metainfo.pieces.begin();it!=metainfo.pieces.end();it++) {
                uint32_t  s;
                if(std::distance(it, metainfo.pieces.end()) == 1) {
                    s = metainfo.length - (metainfo.pieces.size() - 1) * metainfo.piece_length;
                } else {
                    s = metainfo.piece_length;
                }

                pieces.emplace_back(*it, s);
            }

            out_file.resize(size);
        }

    void Torrent::writeBlock(uint64_t piece_index, uint64_t block_offset, const std::vector<uint8_t>& data) {
        uint32_t file_offset = piece_index * nominal_piece_size + block_offset;
        //std::cout << "file off " << file_offset << std::endl;
        Block& block = pieces[piece_index].getBlockByOffset(block_offset);
        block.write_buf = std::move(data);

        boost::asio::async_write_at(
            out_file, file_offset, boost::asio::buffer(block.write_buf), boost::asio::transfer_exactly(data.size()),
            boost::asio::bind_executor(strand, [this, piece_index, block_offset](boost::system::error_code e, std::size_t) {
                this->pieces[piece_index].blockWritten(e, block_offset);
                if(this->pieces[piece_index].complete()) {
                    //std::cout << "Send validate " << piece_index << std::endl;
                    validatePiece(piece_index);
                }
            })
        );
    }

    void Torrent::validatePiece(uint64_t piece_index) {
        //std::cout << "got validate " << piece_index << std::endl;
        Piece& piece = pieces[piece_index];
        uint64_t offset = piece_index * nominal_piece_size;

        piece.validation_buf.resize(piece.size);

        boost::asio::async_read_at(
            out_file, offset, boost::asio::buffer(piece.validation_buf),
            boost::asio::bind_executor(strand, [this, piece_index, &piece](boost::system::error_code, std::size_t r) {
                if(sha1::hashValid(piece.validation_buf, piece.hash)) {
                    this->downloaded += piece.size;
                    std::cout << "Piece validation SUCCESS " << piece_index << std::endl;
                    auto event = std::make_shared<ftorrent::events::PieceVerifiedEvent>(piece);
                    ftorrent::events::Dispatcher::get()->publish(event);
                } else {
                    std::cout << "Piece validation FAIL " << piece_index << std::endl;
                    piece.reset();
                }

                std::vector<uint8_t> empty;
                piece.validation_buf.swap(empty); // clear the buffer
            })
        );
    }
}; // torrent
}; // ftorrent
