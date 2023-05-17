#ifndef FTORRENT_PIECE_PICKER_H
#define FTORRENT_PIECE_PICKER_H

#include <vector>
#include <memory>

#include "../piece.h"
#include "../peer/abstract_peer.h"

namespace ftorrent {
namespace piece_picker {

    class PiecePicker {
    public:
        PiecePicker(const std::vector<std::shared_ptr<Piece>>& pcs): pieces{pcs}, piece_bitfield(pcs.size(), false) {
            for(auto piece : pcs) {
                if(piece->complete()) {
                    piece_bitfield[piece->index] = true;
                }
            }
        }

        virtual void on_have(uint32_t index) = 0;
        virtual void on_bitfield(std::vector<bool> bitfield) = 0;
        virtual std::shared_ptr<Piece> next(std::shared_ptr<peer::AbstractPeer> p, uint32_t attempt) = 0;

        std::vector<bool> get_have_bitfield() const {
            return piece_bitfield;
        }

        void piece_verified(uint32_t index) {
            piece_bitfield[index] = true;
        }

        std::shared_ptr<Piece> get_piece(uint32_t index) {
            return pieces[index];
        }

    protected:
        std::vector<std::shared_ptr<Piece>> pieces;
    private:
        std::vector<bool> piece_bitfield;
    };

}; // piece_picker
}; // ftorrent

#endif
