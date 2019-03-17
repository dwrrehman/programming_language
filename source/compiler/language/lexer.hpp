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

enum class token_type {null, string, identifier, documentation, character, llvm, keyword, operator_, builtin};

struct token {
    enum token_type type = token_type::null;
    std::string value = "";
    size_t line = 0;
    size_t column = 0;
};

void start_lex(std::string given_filename, std::string given_text);

struct token next();

#endif /* lexer_hpp */
