#ifndef JERRY_LEXER
#define JERRY_LEXER

#include <string>
#include <optional>
#include <string>

#include "token.h"

namespace jerry {
    class Lexer {
    public:
        Lexer(std::string source): data{source} {}
        Lexer(const Lexer& other) = delete;
        Lexer& operator=(const Lexer& other) = delete;
        Lexer(Lexer&& other) = delete;
        Lexer& operator=(Lexer&& other) = delete;
        ~Lexer() = default;

        Token current() {
            return *cur_token;
        }

        Token next() {
            extract();
            return current();
        }

        void reset(std::string s) {
            data = s;
            cur_token = std::nullopt;
            index = 0;
        }

    private:
        std::string data;
        std::optional<Token> cur_token = std::nullopt;
        long long index = 0;

        void extract();
        void extractInteger();
        void extractList();
        void extractDict();
        void extractString();

        void step() {
            index++;
        }

        void step(int steps) {
            index += steps;
        }
    };
};


#endif