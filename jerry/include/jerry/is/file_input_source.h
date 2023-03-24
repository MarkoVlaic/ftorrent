#ifndef JERRY_FILE_INPUT_SOURCE
#define JERRY_FILE_INPUT_SOURCE

#include <filesystem>
#include <fstream>
#include <string>

#include "input_source.h"

namespace jerry {
namespace is {

    class FileInputSource : public InputSource {
    public:
        FileInputSource(std::filesystem::path path) {
            input.open(path.c_str(), std::ifstream::binary);
        }

        virtual std::optional<char> get() const override;
        virtual std::optional<std::string> get(int n) const override;
        
        virtual void step() override;
        virtual void step(int steps) override;

        virtual uint64_t getIndex() const override;

        virtual ~FileInputSource() {
            input.close();
        };

    private:
        mutable std::ifstream input;
        uint64_t index = 0;
    };

}; // is
}; // jerry


#endif