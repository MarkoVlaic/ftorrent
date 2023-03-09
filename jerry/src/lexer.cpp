#include <ctype.h>
#include <string>
#include <iostream>

#include "jerry/lexer.h"
#include "jerry/jerry_exception.h"

namespace jerry {
    void Lexer::extract() {
        if(!source->get().has_value()) {
            cur_token = Token{TokenType::END_OF_FILE, "", source->getIndex()};
            return;
        }

        char cur = source->get().value();
        std::cout << "lex cur " << cur << std::endl;
        if(cur == 'i') {
            extractInteger();
            return;
        }

        if(cur == 'l') {
            cur_token = Token{TokenType::LIST_START, "l", source->getIndex()};
            source->step();
            return;
        }

        if(cur == 'd') {
            cur_token = Token{TokenType::DICT_START, "d", source->getIndex()};
            source->step();
            return;
        }

        if(cur == 'e') {
            cur_token = Token{TokenType::END, "e", source->getIndex()};
            source->step();
            return;
        }

        if(isdigit(cur)) {
            extractString();
            return;
        }

        std::string error{"Unexpected character "};
        error += source->get().value();
        throw JerryException{error, source->getIndex()};
    }

    void Lexer::extractInteger() {
        source->step();

        if(!source->get().has_value())
            throw JerryException{"Expected 'e' found EOF", source->getIndex()};

        std::string value;
        uint64_t start = source->getIndex();
        std::optional<char> cur = source->get();

        while(cur.has_value() && cur.value() != 'e') {
            if(!isdigit(cur.value()) && cur.value() != '-') {
                std::string error{"Expected a number, found: "};
                error += cur.value();
                throw JerryException{error, source->getIndex()};
            }

            value += source->get().value();
            source->step();
            cur = source->get();
        }

        if(!source->get().has_value())
            throw JerryException("Expected 'e' found EOF", source->getIndex());

        cur_token = Token{TokenType::INTEGER, value, start};
        source->step();
    }

    void Lexer::extractString() {
        std::string len_str;
        long long start = source->getIndex();
        std::optional<char> cur = source->get();

        while(cur.has_value() && cur.value() != ':') {
            if(!isdigit(cur.value())) {
                std::string error{"Expected a number found: "};
                error += cur.value();
                throw JerryException{error, source->getIndex()};
            }

            len_str += cur.value();
            source->step();
            cur = source->get();
        }

        if(!cur.has_value())
            throw JerryException{"Expected ':' found EOF", source->getIndex()};
        
        long long len = std::stoll(len_str);

        source->step();
        std::optional<std::string> opt_value = source->get(len);

        if(!opt_value.has_value()) {
            std::string error{"string length "};
            error += len_str;
            error += " reaches beyod the end of file";
            throw JerryException{error, start};
        }

        std::cout << "I got substr " << opt_value.value() << std::endl;

        source->step(len);
        cur_token = Token{TokenType::STRING, opt_value.value(), start};
    }
};