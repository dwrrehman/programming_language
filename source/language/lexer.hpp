#ifndef lexer_hpp
#define lexer_hpp

#include "arguments.hpp"
#include "lists.hpp"

#include <string>

enum class token_type {null, string, identifier, character, llvm, keyword, operator_, indent};
namespace lexing_state { enum lexing_state {none, string, string_expression, identifier, llvm_string, comment, multiline_comment, indent}; }

struct token {
    token_type type = token_type::null;
    std::string value = "";
    nat line = 0;
    nat column = 0;
};

struct saved_state {
    nat saved_c = 0;
    nat saved_line = 0;
    nat saved_column = 0;
    lexing_state::lexing_state saved_state = lexing_state::none;
    token saved_current = {};
};

void start_lex(const file& file);

token next();
saved_state save();
void revert(saved_state s);

#endif /* lexer_hpp */
