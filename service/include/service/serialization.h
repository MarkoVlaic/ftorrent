#ifndef FTORRENT_SERIALIZATION_H
#define FTORRENT_SERIALIZATION_H

#include <vector>
#include <algorithm>

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

    template<typename Type>
    void serialize(Type, Serializer&);

    template<>
    void serialize<int32_t>(int32_t num, Serializer& serializer) {
        serializer.reserve(4);

        for(int i=0;i<4;i++) {
            uint8_t byte = (num >> (3-i)*8) & 0xFF;
            serializer.write(byte);
        }
    }

    template<>
    void serialize<int64_t>(int64_t num, Serializer& serializer) {
        serializer.reserve(8);

        for(int i=0;i<8;i++) {
            uint8_t byte = (num >> (7-i)*8) & 0xFF;
            serializer.write(byte);
        }
    }

    class Deserializer {
    public:
        Deserializer(std::vector<uint8_t> d): data(d.begin(), d.end()) {}

        Deserializer(Deserializer const&) = delete;
        void operator=(Deserializer const&) = delete;
        Deserializer(Deserializer&&) = delete;
        Deserializer& operator=(Deserializer&&) = delete;

        std::vector<uint8_t> get(uint32_t bytes) {
            if(offset + bytes >= data.size()) {
                throw ftorrent::Exception{"Deserializer: read out of bounds"};
            }

            std::vector<uint8_t> result;
            result.resize(bytes);

            std::copy(data.begin() + offset, data.begin() + offset + bytes, result.begin());
            offset += bytes;
            return result;
        }
    private:
        std::vector<uint8_t> data;
        int offset = 0;
    };

    template<typename Type>
    void deserialize(Type&, Deserializer&);

    template<>
    void deserialize<int32_t>(int32_t& num, Deserializer& deserializer) {
        std::vector<uint8_t> bytes = deserializer.get(4);
        num = 0;

        for(int i=0;i<4;i++) {
            num |= ((int)bytes[i]) << (3 - i);
        }
    }

    template<>
    void deserialize<int64_t>(int64_t& num, Deserializer& deserializer) {
        std::vector<uint8_t> bytes = deserializer.get(8);
        num = 0;

        for(int i=0;i<8;i++) {
            num |= ((int)bytes[i]) << (7 - i);
        }
    }

}; // serialization
}; // ftorrent

#endif
