//
//  lexer.hpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef lexer_hpp
#define lexer_hpp

#include <string>

enum lexing_state {string_state, identifier_state, number_state, documentation_state};
enum token_type {null_type, string_type, identifier_type, number_type, keyword_type, operator_type, documentation_type};

struct token {
    std::string value;
    enum token_type type;
    size_t line;
    size_t column;
};

void print_lex(const std::vector<struct token> &tokens);
const char* convert_token_type_representation(enum token_type type);

std::vector<struct token> lex(std::string text, bool debug = false);

#endif /* lexer_hpp */
