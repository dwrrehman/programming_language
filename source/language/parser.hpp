#ifndef parser_hpp
#define parser_hpp

#include "nodes.hpp"
#include "arguments.hpp"

expression_list parse(const file& file);
expression parse_expression(const file& file, bool can_be_empty, bool newlines_are_a_symbol);

#endif /* parser_hpp */

