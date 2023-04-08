#ifndef FTORRENT_SERIALIZATION_H
#define FTORRENT_SERIALIZATION_H

#include <vector>
#include <algorithm>
#include <array>
#include <string>

#include "exception.h"

namespace ftorrent {
namespace serialization {
    class Serializer {
    public:
        Serializer() = default;
        Serializer(Serializer const&) = delete;
        void operator=(Serializer const&) = delete;
        Serializer(Serializer&&) = delete;
        Serializer& operator=(Serializer&&) = delete;

        void write(uint8_t byte) {
            if(offset >= data_.size()) {
                throw ftorrent::Exception{"Serializer: not enough space"};
            }

            data_[offset] = byte;
            offset++;
        }

        void reserve(uint32_t bytes) {
            data_.resize(data_.size() + bytes);
        }

        std::vector<uint8_t> data() const {
            return data_;
        }

    private:
        std::vector<uint8_t> data_;
        std::size_t offset = 0;
    };

    struct Serializable {
        virtual void serialize(Serializer&) = 0;
    };

    template<typename SerializablePtr>
    void serialize(SerializablePtr p, Serializer& s) {
        p->serialize(s);
    }

    template<>
    void serialize(uint16_t num, Serializer& serializer);

    template<>
    void serialize(uint32_t num, Serializer& serializer);

    template<>
    void serialize(std::array<uint32_t, 2> arr, Serializer& serializer);

    template<>
    void serialize(std::array<uint8_t, 20> arr, Serializer& serializer);

    template<>
    void serialize(uint64_t num, Serializer& serializer);

    class Deserializer {
    public:
        Deserializer(std::vector<uint8_t> d): data(d.begin(), d.end()) {}

        Deserializer(Deserializer const&) = delete;
        void operator=(Deserializer const&) = delete;
        Deserializer(Deserializer&&) = delete;
        Deserializer& operator=(Deserializer&&) = delete;

        std::vector<uint8_t> get(uint32_t bytes) {
            if(offset + bytes > data.size()) {
                throw ftorrent::Exception{"Deserializer: read out of bounds"};
            }

            uint32_t start_offset = offset;
            offset += bytes;
            return std::vector(data.begin() + start_offset, data.begin() + start_offset + bytes);
        }

        uint8_t get() {
            if(empty()) {
                throw ftorrent::Exception{"Deserializer: read out of bounds"};
            }

            offset += 1;
            return data[offset - 1];
        }

        void reset() {
            offset = 0;
        }

        void reset(const std::vector<uint8_t>& d) {
            data = d;
        }

        bool empty() const {
            return offset >= data.size();
        }

        uint32_t left() const {
            return data.size() - offset;
        }

    private:
        std::vector<uint8_t> data;
        uint32_t offset = 0;
    };

    template<typename Type>
    void deserialize(Type&, Deserializer&);

    template<>
    void deserialize(uint16_t& num, Deserializer& deserializer);

    template<>
    void deserialize(uint32_t& num, Deserializer& deserializer);

    template<>
    void deserialize(uint64_t& num, Deserializer& deserializer);

    template<>
    void deserialize(std::array<uint32_t, 2>& arr, Deserializer& deserializer);

    template<>
    void deserialize(std::string& str, Deserializer& deserializer);

}; // serialization
}; // ftorrent

#endif
