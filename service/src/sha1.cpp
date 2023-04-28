#include "service/sha1.h"
#include "service/types.h"

namespace ftorrent {
namespace sha1 {
  bool hashValid(const std::vector<uint8_t>& data, types::Hash hash) {
      types::Hash data_hash = computeHash(data);
      return std::equal(data_hash.begin(), data_hash.end(), hash.begin());
    }
}; // sha1
}; // ftorrent
