//
//  debug.cpp
//  language
//
//  Created by Daniel Rehman on 1903063.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "debug.hpp"

#include "arguments.hpp"
#include "preprocessor.hpp"
#include "lexer.hpp"
#include "parser.hpp"


#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

// ----------------------- command line arguments debugging: ----------------------------


void debug_arguments(struct arguments args) {
    for (auto a : args.files) {
        std::cout << "file: " << a.name << "\n";
        std::cout << "data: \n:::" << a.text << ":::\n";
    }
    
    std::cout << std::boolalpha;
    std::cout << "error = " << args.error << std::endl;
    std::cout << "use interpreter = " << args.use_interpreter << std::endl;
    std::cout << "exec name = " << args.executable_name << std::endl;
}






// ---------------------------- preprocessor debugging functions: -------------------------------













// ---------------------------- lexer debugging: ---------------------------------------


void print_lex(const std::vector<struct token> &tokens) {
    std::cout << "::::::::::LEX:::::::::::" << std::endl;
    for (auto token : tokens) {
        std::cout << "TOKEN(type: " << convert_token_type_representation(token.type) << ", value: \"" << (token.value != "\n" ? token.value : "\\n") << "\", [" << token.line << ":" << token.column << "])" << std::endl;
    }
    std::cout << ":::::::END OF LEX:::::::" << std::endl;
}

const char* convert_token_type_representation(enum token_type type) {
    switch (type) {
        case token_type::null: return "{null}";
        case token_type::string: return "string";
        case token_type::identifier: return "identifier";
        case token_type::keyword: return "keyword";
        case token_type::operator_: return "operator";
        case token_type::documentation: return "documentation";
        case token_type::character: return "character";
        case token_type::llvm: return "llvm";
        case token_type::builtin: return "builtin";
        case token_type::indent: return "indent";
    }
}

void debug_token_stream() {
    std::vector<struct token> tokens = {};
    struct token t = {};
    while ((t = next()).type != token_type::null) tokens.push_back(t);
    print_lex(tokens);
}



// ---------------------------- parser -------------------------------


#define prep(_level) for (int i = _level; i--;) std::cout << ".   "


void print_expression_list(expression_list list) {
    std::cout << "pritning expression list:\n";
    for (auto e : list.expressions) {
        std::cout << "length: " << e.symbols.size() << "\n";
        std::cout << std::boolalpha << "error: " << e.error << "\n";
        std::cout << "\n";
    }
}
