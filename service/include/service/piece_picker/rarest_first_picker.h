#ifndef FTORRENT_RAREST_FIRST_PICKER
#define FTORRENT_RAREST_FIRST_PICKER

#include <map>
#include <set>

#include "./piece_picker.h"
#include "../piece.h"
#include "../peer/abstract_peer.h"

namespace ftorrent {
namespace piece_picker {
    class RarestFirstPicker : public PiecePicker {
    public:
        RarestFirstPicker(const std::vector<std::shared_ptr<Piece>>& pcs);
        ~RarestFirstPicker() override = default;

        void on_have(uint32_t index) override;
        void on_bitfield(std::vector<bool> bitfield) override;
        std::shared_ptr<Piece> next(std::shared_ptr<peer::AbstractPeer> p, uint32_t attempt) override;
    private:
        void sort_pieces();
        void increment_piece_availability(uint32_t index);
    private:
        std::vector<std::vector<std::shared_ptr<Piece>>> availability_buckets;
        bool dirty = true;
    };

};
};

#endif
