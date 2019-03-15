//
//  debug.hpp
//  language
//
//  Created by Daniel Rehman on 1903063.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef debug_hpp
#define debug_hpp

#include "arguments.hpp"
#include "preprocessor.hpp"
#include "lexer.hpp"
#include "parser.hpp"

#include <vector>

// cli arguments
void debug_arguments(struct arguments args);

// preprocessor:
const char* convert_pp_token_type_representation(enum pp_token_type type);
void print_pp_lex(const std::vector<struct pp_token> &tokens);

// lexer:
void print_lex(const std::vector<struct token> &tokens);
const char* convert_token_type_representation(enum token_type type);

// parser:
void print_node(node &node, int level);
void print_token(struct token t);
void print_parse(node &tree);

#endif /* debug_hpp */
