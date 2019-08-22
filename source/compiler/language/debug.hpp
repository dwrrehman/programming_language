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

#include "lexer.hpp" 
#include "nodes.hpp"

#include <vector>

#define prep(_level) for (int i = _level; i--;) std::cout << ".   "

// cli arguments
void debug_arguments(struct arguments args);

// lexer:
void print_lex(const std::vector<struct token> &tokens);
const char* convert_token_type_representation(enum token_type type);
void debug_token_stream();

// parser:

std::string convert_symbol_type(enum symbol_type type);

void print_symbol(symbol s, int d);
void print_expression(expression s, int d);
void print_block(block b, int d);
void print_expression_list(expression_list list, int d);
void print_expression_list_line(expression_list list);
void print_translation_unit(translation_unit unit, struct file file);
void print_abstraction_definition(abstraction_definition abstraction, int d);
void print_expression_line(expression expression);
void print_block_line(block block);

#endif
