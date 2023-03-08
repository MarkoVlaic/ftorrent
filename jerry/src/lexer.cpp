#include <ctype.h>
#include <string>

#include "jerry/lexer.h"
#include "jerry/jerry_exception.h"

namespace jerry {
    void Lexer::extract() {
        if(index >= data.size()) {
            cur_token = Token{TokenType::END_OF_FILE, "", index};
            return;
        }

        char cur = data[index];

        if(cur == 'i') {
            extractInteger();
            return;
        }

        if(cur == 'l') {
            cur_token = Token{TokenType::LIST_START, "l", index};
            step();
            return;
        }

        if(cur == 'd') {
            cur_token = Token{TokenType::DICT_START, "d", index};
            step();
            return;
        }

        if(cur == 'e') {
            cur_token = Token{TokenType::END, "e", index};
            step();
            return;
        }

        if(isdigit(cur)) {
            extractString();
            return;
        }

        std::string error{"Unexpected character "};
        error += data[index];
        throw JerryException{error, index};
    }

    void Lexer::extractInteger() {
        step();

        if(index >= data.size())
            throw JerryException{"Expected 'e' found EOF", index};

        long long start = index;

        while(index < data.size() && data[index] != 'e') {
            if(!isdigit(data[index]) && data[index] != '-') {
                std::string error{"Expected a number, found: "};
                error += data[index];
                throw JerryException{error, index};
            }
            step();
        }

        if(index >= data.size())
            throw JerryException("Expected 'e' found EOF", index);

        std::string value = data.substr(start, index - start);
        cur_token = Token{TokenType::INTEGER, value, start};

        step();
    }

    void Lexer::extractString() {
        long long start = index;
        while(index < data.size() && data[index] != ':') {
            if(!isdigit(data[index])) {
                std::string error{"Expected a number found: "};
                error += data[index];
                throw JerryException{error, index};
            }

            step();
        }

        if(index >= data.size())
            throw JerryException{"Expected ':' found EOF", index};
        
        long long len = std::stoll(data.substr(start, index - start));
        std::string value = data.substr(index + 1, len);
        step(len + 1);
        
        cur_token = Token{TokenType::STRING, value, start};
    }
};