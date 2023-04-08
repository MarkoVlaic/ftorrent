#include "service/serialization.h"
#include "service/util.h"

#include <iostream>

namespace ftorrent {
namespace serialization {
    /*template<typename Type>
    void serialize(Type t, Serializer& s) {
        t.serialize(s);
    }*/

    template<>
    void serialize(uint16_t num, Serializer& serializer) {
        serializer.reserve(2);

        for(int i=0;i<2;i++) {
            uint8_t byte = (num >> i*8) & 0xFF;
            serializer.write(byte);
        }
    }

    template<>
    void serialize(uint32_t num, Serializer& serializer) {
        serializer.reserve(4);

        for(int i=0;i<4;i++) {
            uint8_t byte = (num >> i*8) & 0xFF;
            serializer.write(byte);
        }
    }

    template<>
    void serialize(std::array<uint32_t, 2> arr, Serializer& serializer) {
        serializer.reserve(8);
        serialize(arr[0], serializer);
        serialize(arr[1], serializer);
    }

    template<>
    void serialize(uint64_t num, Serializer& serializer) {
        serializer.reserve(8);

        for(int i=0;i<8;i++) {
            uint8_t byte = (num >> i*8) & 0xFF;
            serializer.write(byte);
        }
    }

    template<>
    void serialize(std::array<uint8_t, 20> arr, Serializer& serializer) {
        serializer.reserve(20);
        for(uint8_t byte : arr) {
            serializer.write(byte);
        }
    }

    template<>
    void deserialize(uint16_t& num, Deserializer& deserializer) {
        std::vector<uint8_t> bytes = deserializer.get(2);
        num = bytes[0] | (((uint16_t)bytes[1]) << 8);
    }

    template<>
    void deserialize(uint32_t& num, Deserializer& deserializer) {
        std::vector<uint8_t> bytes = deserializer.get(4);
        num = 0;

        for(int i=0;i<4;i++) {
            unsigned int large_byte = bytes[i];
            num |= large_byte << (i * 8);
        }
    }

    template<>
    void deserialize(uint64_t& num, Deserializer& deserializer) {
        std::vector<uint8_t> bytes = deserializer.get(8);

        std::cout << std::hex << "int64t bytes: ";
        for(auto byte : bytes) {
            std::cout << (int)byte << " ";
        }
        std::cout << "\n";

        num = 0;

        for(int i=0;i<8;i++) {
            uint64_t large_byte = bytes[i];
            num |= large_byte << (i * 8);
        }
    }

    template<>
    void deserialize(std::array<uint32_t, 2>& arr, Deserializer& deserializer) {
        deserialize(arr[0], deserializer);
        deserialize(arr[1], deserializer);
    }


    template<>
    void deserialize(std::string& str, Deserializer& deserializer) {
        str = "";
        while(!deserializer.empty()) {
            char cur = deserializer.get();
            if(cur == 0)
                return;

            str.append(1, cur);
        }
    }
};
};
