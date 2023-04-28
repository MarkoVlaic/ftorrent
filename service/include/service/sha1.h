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

#include "types.h"

namespace ftorrent {
namespace sha1 {
    bool hashValid(const std::vector<uint8_t>& data, types::Hash hash);

    template<typename DataContainer>
    types::Hash computeHash(const DataContainer& data) {
        types::Hash result;
        digestpp::sha1().absorb(data.begin(), data.end()).digest(result.data());

        return result;
    }
}; // sha1
}; // ftorrent

#endif //ZAVRSNI_SHA1_H
