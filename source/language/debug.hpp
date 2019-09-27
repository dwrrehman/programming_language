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
#include "analysis_ds.hpp"

#include <vector>

#define prep(_level) for (nat i = _level; i--;) std::cout << ".   "

// cli arguments
void debug_arguments(const arguments& args);

// lexer:
void print_lex(const std::vector<token>& tokens);
const char* convert_token_type_representation(token_type type);
void debug_token_stream();

// parser:

std::string convert_symbol_type( symbol_type type);
void print_symbol(symbol s, int d);
void print_expression(expression s, int d);
void print_expression_list(expression_list list, int d);
void print_expression_list_line(expression_list list);
void print_expression_line(expression expression);
void print_translation_unit(expression_list unit, file file);

//void debug_table(const llvm_module& module, symbol_table& stack);

void print_resolved_expr(resolved_expression expr, nat depth, state& state);
void print_resolved_list(resolved_expression_list list, nat depth, state& state);

void print_resolved_unit(resolved_expression_list unit, state& state);

void print_nat_vector(std::vector<nat> v, bool newline); 

#endif
