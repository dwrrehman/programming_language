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

enum class token_type {null, string, identifier, documentation, character, llvm, keyword, operator_, indent};
enum class lexing_state {none, string, string_expression, identifier, documentation, character_or_llvm, comment, multiline_comment, indent};

struct token {
    enum token_type type = token_type::null;
    std::string value = "";
    size_t line = 0;
    size_t column = 0;
};

struct saved_state {
    size_t saved_c = 0;
    size_t saved_line = 0;
    size_t saved_column = 0;
    lexing_state saved_state = lexing_state::none;
    struct token saved_current = {};
};

void start_lex(const struct file file);

struct token next();
struct saved_state save();
void revert(struct saved_state s);

#endif /* lexer_hpp */
