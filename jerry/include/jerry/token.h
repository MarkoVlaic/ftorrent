#ifndef JERRY_TOKEN
#define JERRY_TOKEN

#include "data.h"
#include <string>

namespace jerry {

    enum TokenType {
        STRING,
        INTEGER,
        LIST_START,
        DICT_START,
        END,
        END_OF_FILE
    };

    struct Token {
        Token(TokenType t, std::string v, uint64_t i): type{t}, value{v}, index{i} {};
        Token(const Token& rhs) = default;
        Token& operator=(const Token& other) = default;
        Token(Token&& other) = default;
        Token& operator=(Token&& other) = default;
        ~Token() = default;
        
        TokenType type;
        std::string value;
        uint64_t index;
    };

};

#endif