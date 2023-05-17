#include <algorithm>
#include <iostream> // TODO remove

#include "service/piece_picker/rarest_first_picker.h"
#include "service/piece.h"

namespace ftorrent {
namespace piece_picker {

    RarestFirstPicker::RarestFirstPicker(const std::vector<std::shared_ptr<Piece>>& pcs): PiecePicker{pcs}
    {}

    struct AvailabilityCmp {
        bool operator()(const std::shared_ptr<Piece>& lhs, const std::shared_ptr<Piece>& rhs) const {
            return lhs->get_availability() < rhs->get_availability();
        }
    };

    void RarestFirstPicker::on_have(uint32_t index) {
        increment_piece_availability(index);
        sort_pieces();
    }

    void RarestFirstPicker::on_bitfield(std::vector<bool> bitfield) {
        for(int i=0;i<bitfield.size();i++) {
            if(bitfield[i]) {
                increment_piece_availability(i);
            }
        }

        sort_pieces();
    }

    void RarestFirstPicker::increment_piece_availability(uint32_t index) {
        auto piece = *std::find_if(pieces.begin(), pieces.end(), [index](std::shared_ptr<Piece> piece){
            return piece->index == index;
        });
        piece->set_availability(piece->get_availability() + 1);
    }

    std::shared_ptr<Piece> RarestFirstPicker::next(std::shared_ptr<peer::AbstractPeer> p, uint32_t attempt) {
        for(auto bucket : availability_buckets) {
            for(auto piece : bucket) {
                if(piece->get_availability() == 0) {
                    continue;
                }

                if(!piece->complete() && p->has_piece(piece->index)) {
                    if(attempt == 0) {
                        return piece;
                    }
                    attempt--;
                }
            }
        }

        return nullptr;
    }

    void RarestFirstPicker::sort_pieces() {
        std::sort(pieces.begin(), pieces.end(), AvailabilityCmp{});

        availability_buckets.clear();
        std::vector<std::shared_ptr<Piece>> bucket;
        int availability = pieces[0]->get_availability();

        for(auto it=pieces.begin();it!=pieces.end();it++) {
            if((*it)->get_availability() != availability) {
                std::random_shuffle(bucket.begin(), bucket.end());
                availability_buckets.push_back(bucket);
                bucket.clear();
                availability = (*it)->get_availability();
            }

            bucket.push_back(*it);
        }
        availability_buckets.push_back(bucket);
    }
};
};
