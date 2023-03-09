#ifndef JERRY_STRING_INPUT_SOURCE
#define JERRY_STRING_INPUT_SOURCE

#include <string>

#include "input_source.h"

namespace jerry {
namespace is {

    class StringInputSource : public InputSource {
    public:
        StringInputSource(): StringInputSource{""} {}
        StringInputSource(const std::string& s): data{s} {}

        virtual std::optional<char> get() const override;
        virtual std::optional<std::string> get(int n) const override;
        
        virtual void step() override;
        virtual void step(int steps) override;

        virtual uint64_t getIndex() const override;

        virtual ~StringInputSource() = default;
    private:
        std::string data;
        uint64_t index = 0;
    };

}; // is
}; // jerry

#endif