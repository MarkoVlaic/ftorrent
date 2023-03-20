#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <map>
#include <variant>
#include <stdexcept>
#include <memory>

#include "jerry/token.h"
#include "jerry/lexer.h"
#include "jerry/parser.h"
#include "jerry/jerry.h"

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

std::string PrintTokenType(const jerry::TokenType& type) {
    switch (type)
    {
    case jerry::TokenType::STRING:
        return "STRING";
    case jerry::TokenType::INTEGER:
        return "INTEGER";
    case jerry::TokenType::LIST_START:
        return "LIST_START";
    case jerry::TokenType::DICT_START:
        return "DICT_START";
    case jerry::TokenType::END:
        return "END";
    case jerry::TokenType::END_OF_FILE:
        return "EOF";
    default:
        throw std::runtime_error{"Unknown TokenType"};
    }
}

std::string read_whole_file(std::ifstream& is) {
    constexpr size_t read_size = 4 * 1024;
    is.exceptions(std::ios::badbit);
    
    std::string result;
    std::string buf(read_size, '\0');

    while(is.read(&buf[0], read_size)) {
        result.append(buf, 0, is.gcount());
    }
    result.append(buf, 0, is.gcount());

    return result;
}

void DemoStringSource(char* source_path) {
    std::ifstream sourceFile{source_path, std::ios::binary};
    std::string source{read_whole_file(sourceFile)};

    std::cout << sourceFile.good() << std::endl;
    std::cout << source << std::endl;
    std::cout << "bytes: " << source.size() << std::endl << std::endl << std::endl;

    jerry::Lexer lexer{source};

    while(lexer.next().type != jerry::TokenType::END_OF_FILE) {
        std::cout << PrintTokenType(lexer.current().type) << ", " << lexer.current().value << std::endl;
    }

    std::cout << std::endl;

    jerry::Parser parser{source};
    jerry::Data data = parser.parse();

    std::cout << "I hold " << data.index() << std::endl;
    std::visit(PrintVisitor{}, data);
}

void DemoFileSource(std::filesystem::path path) {
    jerry::Lexer lexer{path};

    while(lexer.next().type != jerry::TokenType::END_OF_FILE) {
        std::cout << PrintTokenType(lexer.current().type) << ", " << lexer.current().value << std::endl;
    }

    std::cout << std::endl;

    jerry::Parser parser{path};
    jerry::Data data = parser.parse();

    std::cout << "I hold " << data.index() << std::endl;
    std::visit(PrintVisitor{}, data);
}

int main(int argc, char** argv) {
    std::cout << argv[0] << std::endl;

    if(argc != 2) {
        std::cout << "usage: main <filename>";
        return 1;
    }

    std::string path_str{argv[1]};
    std::filesystem::path path{path_str};

    std::cout << "Path is: " << path.string() << std::endl << std::endl;

    /*jerry::Data decoded = jerry::bdecode(path);

    std::cout << "back to decode:" << std::endl;
    std::string encoded = jerry::bencode(decoded);
    std::cout.write(encoded.c_str(), encoded.size());*/

    //DemoFileSource(path);

    jerry::Data decoded = jerry::bdecode(path);
    std::visit(PrintVisitor{}, decoded);

    // std::cout << encoded;
}