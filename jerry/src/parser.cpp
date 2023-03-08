#include <string>
#include <vector>
#include <map>

#include "jerry/parser.h"
#include "jerry/data.h"
#include "jerry/jerry_exception.h"

namespace jerry {
    Data Parser::parse() {
        Token current = lexer.current();

        switch (current.type)
        {
        case TokenType::INTEGER:
            return std::stoll(current.value);
        case TokenType::STRING:
            return current.value;
        case TokenType::LIST_START:
            return parseList();
        case TokenType::DICT_START:
            return parseDict();
        default:
            std::string error{"unexpected token "};
            error += current.value;
            throw JerryException(error, current.index);
        }
    }

    Data Parser::parseList() {
        jerry::List list;

        while(lexer.next().type != TokenType::END) {
            list.push_back(parse());
        }

        return list;
    }

    Data Parser::parseDict() {
        jerry::Dict dict;

        while(lexer.next().type != TokenType::END) {
            Token current = lexer.current();

            if(current.type != TokenType::STRING) {
                std::string error{"expected a string, found: "};
                error += current.value;
                throw JerryException{error, current.index};
            }

            std::string key = current.value;
            lexer.next();

            dict[key] = parse();
        }

        return dict;
    }
};