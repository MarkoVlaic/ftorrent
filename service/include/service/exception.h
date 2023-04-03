//
// Created by marko on 3/27/23.
//

#ifndef ZAVRSNI_EXCEPTION_H
#define ZAVRSNI_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace ftorrent {
    struct Exception : std::exception {
        Exception(const std::string& msg): message_{msg} {}
        const char* what() const noexcept override {
            return message_.c_str();
        }

    private:
        std::string message_;
    };
}; // ftorrent

#endif //ZAVRSNI_EXCEPTION_H
