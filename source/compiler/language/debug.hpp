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

// preprocessor:
const char* convert_pp_token_type_representation(enum pp_token_type type);
void print_pp_lex(const std::vector<struct pp_token> &tokens);
void print_pp_node(pp_node &self, int level);
void print_pp_parse(pp_node &tree);
void print_value(struct value v);
void print_current_symbol_table(std::unordered_map<std::string, struct value> symbol_table);
void print_symbol_table_stack(std::vector<std::unordered_map<std::string, struct value>> symbol_table_stack);

// lexer:
void print_lex(const std::vector<struct token> &tokens);
const char* convert_token_type_representation(enum token_type type);

// parser:
void print_node(node &node, int level);
void print_token(struct token t);
void print_parse(node &tree);

#endif /* debug_hpp */
