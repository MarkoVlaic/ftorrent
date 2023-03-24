#include <optional>
#include <string>

#include "jerry/is/string_input_source.h"

namespace jerry {
namespace is {
    std::optional<char> StringInputSource::get() const {
        if(index >= data.size())
            return std::nullopt;
        
        return data[index];
    }

    std::optional<std::string> StringInputSource::get(int n) const {
        if(index + n >= data.size())
            return std::nullopt;
        
        return data.substr(index, n);
    }

    void StringInputSource::step() {
        index++;
    }

    void StringInputSource::step(int steps) {
        index += steps;
    }

    uint64_t StringInputSource::getIndex() const {
        return index;
    }
}; // is
}; // jerry