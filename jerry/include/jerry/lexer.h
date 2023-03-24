#ifndef JERRY_LEXER
#define JERRY_LEXER

#include <string>
#include <optional>
#include <utility>
#include <string>
#include <memory>
#include <filesystem>

#include "token.h"
#include "is/input_source.h"
#include "is/string_input_source.h"
#include "is/file_input_source.h"

namespace jerry {
    class Lexer {
    public:
        Lexer(std::string s): source{std::make_unique<is::StringInputSource>(s)} {}
        Lexer(std::filesystem::path p): source{std::make_unique<is::FileInputSource>(p)} {}
        
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

    private:
        std::unique_ptr<is::InputSource> source;
        std::optional<Token> cur_token = std::nullopt;

        void extract();
        void extractInteger();
        void extractString();
    };
};


#endif