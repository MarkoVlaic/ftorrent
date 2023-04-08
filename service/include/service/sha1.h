//
// Created by marko on 3/27/23.
//

#ifndef ZAVRSNI_SHA1_H
#define ZAVRSNI_SHA1_H

#include <array>
#include <algorithm>
#include <cstdint>
#include <vector>
#include <iostream>
#include "digestpp.hpp"

namespace ftorrent {
namespace sha1 {
    using Hash = std::array<uint8_t, 20>;

    bool hashValid(const std::vector<uint8_t>& data, Hash hash);

    template<typename DataContainer>
    Hash computeHash(const DataContainer& data) {
        Hash result;
        digestpp::sha1().absorb(data.begin(), data.end()).digest(result.data());

        return result;
    }
}; // sha1
}; // ftorrent

#endif //ZAVRSNI_SHA1_H
