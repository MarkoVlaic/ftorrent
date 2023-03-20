#include <filesystem>
#include <string>

#include "jerry/data.h"
#include "jerry/token.h"
#include "jerry/lexer.h"
#include "jerry/parser.h"
#include "jerry/jerry_exception.h"
#include"jerry/jerry.h"

namespace jerry {
    Data bdecode(std::filesystem::path path) {
        Parser parser{path};
        return parser.parse();
    }

    Data bdecode(std::string s) {
        Parser parser{s};
        return parser.parse();
    }

    namespace {
        struct EncodeVisitor {
            std::string content;

            void operator()(long long i) {
                content = "i";
                content += std::to_string(i);
                content += "e";
            }

            void operator()(const std::string& s) {
                content = std::to_string(s.size());
                content += ":";
                content += s;
            }

            void operator()(const jerry::List& l) {
                content = "l";
                
                for(auto elem : l) {
                    EncodeVisitor elemVisitor;
                    std::visit(elemVisitor, elem);
                    content += elemVisitor.content;
                }

                content += "e";
            }

            void operator()(const jerry::Dict& d) {
                content += "d";

                for(auto kv : d) {
                    EncodeVisitor keyVisitor;
                    Data keyData = kv.first;
                    std::visit(keyVisitor, keyData);
                    content += keyVisitor.content;

                    EncodeVisitor valueVisitor;
                    std::visit(valueVisitor, kv.second);
                    content += valueVisitor.content;
                }
            }
        };
    }; // namespace

    std::string bencode(Data d) {
        EncodeVisitor visitor;
        std::visit(visitor, d);

        return visitor.content;
    }

}; // jerry