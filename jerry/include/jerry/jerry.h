#ifndef JERRY_H
#define JERRY_H

#include <filesystem>
#include "data.h"

namespace jerry {
    Data bdecode(std::filesystem::path path);
    Data bdecode(std::string s);
    std::string bencode(Data d);
};

#endif