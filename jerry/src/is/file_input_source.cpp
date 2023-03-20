#include <optional>
#include <iostream>
#include <memory>
#include <string>

#include "jerry/is/file_input_source.h"
#include "jerry/jerry_exception.h"

namespace jerry {
namespace is {

    std::optional<char> FileInputSource::get() const {
        char ret = (char) input.peek();

        if(input.eof())
            return std::nullopt;

        if(!input)
            throw JerryException{"error while reading file", index};

        return ret;
    }

    std::optional<std::string> FileInputSource::get(int n) const {
        std::cout << "read n " << n << std::endl;
        std::unique_ptr<char[]> buf(new char[n + 1]);
        input.read(buf.get(), n);

        if(input.eof() || input.gcount() < n)
            return std::nullopt;

        if(!input)
            throw JerryException{"error while reading file", index};

        buf.get()[n] = '\0';
        std::cout << "read " << buf.get() << " read len " << input.gcount() << std::endl;

        input.seekg(-n, input.cur);

        return std::string{buf.get()};
    }

    void FileInputSource::step() {
        step(1);
    }

    void FileInputSource::step(int n) {
        index += n;
        input.ignore(n);
    }

    uint64_t FileInputSource::getIndex() const {
        return index;
    }

}; // is
}; // jerry