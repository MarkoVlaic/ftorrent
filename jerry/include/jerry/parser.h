#ifndef JERRY_PARSER
#define JERRY_PARSER

#include <string>

#include "lexer.h"
#include "data.h"

namespace jerry {
    class Parser {
    public:
        Parser(std::string source): lexer{Lexer{source}} {
            lexer.next();
        }
        Data parse();
    private:
        Lexer lexer;

        Data parseList();
        Data parseDict();
    };
};

#endif