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
#include <unordered_map>
#include <string>

#define prep(_level) for (int i = _level; i--;) std::cout << ".   "

// cli arguments
void debug_arguments(struct arguments args);


// lexer:
void print_lex(const std::vector<struct token> &tokens);
const char* convert_token_type_representation(enum token_type type);
void debug_token_stream();

// parser:
void print_expression_list(expression_list list);

#endif /* debug_hpp */
