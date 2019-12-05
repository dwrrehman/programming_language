#ifndef debug_hpp
#define debug_hpp

#include "analysis_ds.hpp"

#include <vector>

#define prep(_level) for (nat i = _level; i--;) std::cout << ".   "

void debug_arguments(const arguments& args);

void print_lex(const std::vector<token>& tokens);
const char* convert_token_type_representation(token_type type);
void debug_token_stream();

std::string convert_symbol_type( symbol_type type);
void print_symbol(symbol s, nat d);
void print_expression(expression s, nat d);
void print_expression_list(expression_list list, nat d);
void print_expression_list_line(expression_list list);
void print_expression_line(expression expression);
void print_translation_unit(expression_list unit, file file);

void print_resolved_expr(resolved_expression expr, nat depth, state& state);
void print_resolved_list(resolved_expression_list list, nat depth, state& state);
void print_resolved_unit(resolved_expression_list unit, state& state);
void print_nat_vector(std::vector<nat> v, bool newline);
void debug_table(symbol_table table);
#endif
