
#include "service/sha1.h"
#include <iostream>

namespace ftorrent {
namespace sha1 {
  bool hashValid(const std::vector<uint8_t>& data, Hash hash) {
      Hash data_hash = computeHash(data);
      return std::equal(data_hash.begin(), data_hash.end(), hash.begin());
    }
}; // sha1
}; // ftorrent
