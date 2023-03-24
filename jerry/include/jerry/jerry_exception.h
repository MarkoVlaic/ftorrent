#include <sstream>
#include <stdexcept>

namespace jerry {
    class JerryException : public std::exception {
    public:
        JerryException(std::string msg, uint64_t index) {
            std::ostringstream msg_stream{"JerryException at ", std::ios::ate};
            msg_stream << index << ": " << msg;
            err = msg_stream.str();
        }

        virtual const char* what() const throw () {
        return err.c_str();
        }
        
    private:
        std::string err;
    };
}