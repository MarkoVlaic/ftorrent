#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <variant>

#include "jerry/token.h"
#include "jerry/lexer.h"
#include "jerry/parser.h"

struct PrintVisitor {
    int indentation = 0;
    std::string whitespace;

    PrintVisitor(): PrintVisitor(0){}
    PrintVisitor(int i): indentation{i} {
        whitespace = std::string(indentation, ' ');
    }

    void operator()(long long i) const {
        std::cout << whitespace << i;
    }

    void operator()(const std::string& s) const {
        std::cout << whitespace << s;
    }

    void operator()(const jerry::List& v) const {
        std::cout << whitespace << "[" << std::endl;

        for(jerry::Data d : v) {
            // int i = std::holds_alternative<jerry::List>(d) || std::holds_alternative<jerry::Dict>(d) ? indentation + 1 : indentation;
            std::visit(PrintVisitor{ indentation + 1 }, d);
            std::cout << whitespace << "," << std::endl;
        }

        std::cout << whitespace << "]" << std::endl;
    }

    void operator()(const jerry::Dict& m) const {
        std::cout << whitespace << "{" << std::endl;

        for(auto it = m.begin();it != m.end();it++) {
            std::cout << whitespace << it->first << ":";
            int i = std::holds_alternative<jerry::List>(it->second) || std::holds_alternative<jerry::Dict>(it->second) ? indentation + 1 : indentation;
            std::visit(PrintVisitor{ indentation + 1 }, it->second);

            if(std::distance(it, m.end()) != 1)
                std::cout << whitespace << "," ;

            std::cout << std::endl;
        }

        std::cout << whitespace << "}" << std::endl;
    }
};

int main(int argc, char** argv) {
    std::cout << argv[0] << std::endl;

    if(argc != 2) {
        std::cout << "usage: main <filename>";
        return 1;
    }

    std::ifstream sourceFile{argv[1]};
    std::stringstream buffer;
    buffer << sourceFile.rdbuf();
    std::string source{buffer.str()};

    std::cout << sourceFile.good() << std::endl;
    std::cout << source << std::endl;

    jerry::Lexer lexer{source};

    while(lexer.next().type != jerry::TokenType::END_OF_FILE) {
        std::cout << lexer.current().type << ", " << lexer.current().value << std::endl;
    }

    std::cout << std::endl;

    jerry::Parser parser{source};
    jerry::Data data = parser.parse();

    std::cout << "I hold " << data.index() << std::endl;
    std::visit(PrintVisitor{}, data);

}