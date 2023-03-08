#ifndef JERRY_DATA
#define JERRY_DATA

#include <variant>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace jerry {
    struct Data;

    using List = std::vector<Data>;
    using Dict = std::map<std::string, Data>;

    struct Data : std::variant<long long, std::string, List, Dict> {
        using variant::variant;

        variant& base() {
            return *this;
        }
    };
};

#endif