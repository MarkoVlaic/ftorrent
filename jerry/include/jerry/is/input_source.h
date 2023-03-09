#ifndef JERRY_INPUT_SOURCE
#define JERRY_INPUT_SOURCE

#include <string>
#include <optional>

namespace jerry {
namespace is {
    
    class InputSource {
    public:
        InputSource() = default;

        virtual std::optional<char> get() const = 0;
        virtual std::optional<std::string> get(int n) const = 0;
        virtual void step() = 0;
        virtual void step(int steps) = 0;
        virtual uint64_t getIndex() const = 0;
        virtual ~InputSource() = default;
    };

} // is
}; // jerry

#endif