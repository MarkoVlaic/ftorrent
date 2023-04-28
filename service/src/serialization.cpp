#include "service/serialization.h"
#include "service/util.h"

#include <iostream>
#include <algorithm>

namespace ftorrent {
namespace serialization {
    /*template<typename Type>
    void serialize(Type t, Serializer& s) {
        t.serialize(s);
    }*/

    template<>
    void serialize(uint8_t byte, Serializer& serializer) {
        serializer.reserve(1);
        serializer.write(byte);
    }

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
    void serialize(std::array<uint8_t, 8> arr, Serializer& serializer) {
        serializer.reserve(8);
        for(uint8_t byte : arr) {
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
    void serialize(std::string s, Serializer& serializer) {
        serializer.reserve(s.size());
        for(uint8_t ch : s) {
            serializer.write(ch);
        }
    }

    template<>
    void serialize(std::vector<uint8_t> v, Serializer& serializer) {
        serializer.reserve(v.size());
        for(auto byte : v) {
            serializer.write(byte);
        }
    }

    template<>
    void serialize(std::vector<bool> v, Serializer& serializer) {
        uint32_t blocks = v.size() / 8 + (v.size() % 8 > 0);
        serializer.reserve(blocks);

        for(int i=0;i<blocks;i++) {
            uint8_t block = 0;
            unsigned long start_index = i*8;
            unsigned long end_index = std::min(v.size(), start_index+8);

            for(int j=start_index;j<end_index;j++) {
                uint8_t bit_val = v[j] ? 1 : 0;
                block |= (1u << (7 - (j - start_index))) * bit_val;
            }

            serializer.write(block);
        }
    }

    template<>
    void deserialize(uint8_t& byte, Deserializer& deserializer) {
        byte = deserializer.get();
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
    void deserialize(std::array<uint8_t, 8>& arr, Deserializer& deserializer) {
        for(int i=0;i<8;i++) {
            deserialize(arr[i], deserializer);
        }
    }

    template<>
    void deserialize(std::array<uint8_t, 20>& arr, Deserializer& deserializer) {
        for(int i=0;i<20;i++) {
            deserialize(arr[i], deserializer);
        }
    }

    template<>
    void deserialize(std::string& str, Deserializer& deserializer) {
        // TODO: seriously think about changing this behaviour
        str = "";
        while(!deserializer.empty()) {
            char cur = deserializer.get();
            if(cur == 0)
                return;

            str.append(1, cur);
        }
    }

    template<>
    void deserialize(std::vector<uint8_t>& buf, Deserializer& deserializer) {
        for(int i=0;i<buf.size();i++) {
            buf[i] = deserializer.get();
        }
    }
};
};
