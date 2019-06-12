//
//  corrector.cpp
//  language
//
//  Created by Daniel Rehman on 1903192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "corrector.hpp"
#include "nodes.hpp"
#include "parser.hpp"
#include "arguments.hpp"
#include "debug.hpp"
#include "lists.hpp"

/*
        known bug:

            when we give a indent level of 2, it only finds 1 block from it...?

 */

void remove_whitespace_in_expressions(expression_list& list, struct file file, size_t depth) {    
    for (auto& expression : list.expressions) {
        expression.symbols.erase(std::remove_if(
            expression.symbols.begin(), 
            expression.symbols.end(),
            [](symbol e) {
                return e.type == symbol_type::indent 
                    or e.type == symbol_type::newline;            
            }
        ));
        for (auto& symbol : expression.symbols) 
            if (symbol.type == symbol_type::block) 
                remove_whitespace_in_expressions(symbol.block.list, file, depth);                    
    }    
} 

void turn_indents_into_blocks(expression_list& list, struct file file, const size_t level);

void add_block_to_list(block& block, struct file file, const size_t level, expression_list& new_list) {
    turn_indents_into_blocks(block.list, file, level + 1);
    symbol s {symbol_type::block};
    s.block = block;
    expression e {};
    e.error = false;
    e.symbols.push_back(s);
    if (new_list.expressions.empty()) new_list.expressions.push_back(e);
    else new_list.expressions.back().symbols.push_back(s);
}

static void push_block_onto_list(block& block, const struct file& file, const size_t level, expression_list& new_list) {
    if (block.list.expressions.size())
        add_block_to_list(block, file, level, new_list);
}

void turn_indents_into_blocks(expression_list& list, struct file file, const size_t level) {

    expression_list new_list {};
    block block {};

    for (auto& expression : list.expressions) {
        if (expression.symbols.empty()) continue;
        
        if (expression.indent_level > level) {
            block.list.expressions.push_back(expression);
        } else {
            push_block_onto_list(block, file, level, new_list);
            new_list.expressions.push_back(expression);
            block.list.expressions.clear();
        }
    }
    push_block_onto_list(block, file, level, new_list);
    list = new_list;
}

void raise(size_t& value, const size_t minimum) {
    if (value < minimum) value = minimum;
}

void raise_indents(expression_list& list, struct file file, const size_t level) {
    for (auto& expression : list.expressions) {
        raise(expression.indent_level, level);
        for (auto& symbol : expression.symbols)
            if (symbol.type == symbol_type::block)
                raise_indents(symbol.block.list, file, level + 1);
    }
}

translation_unit correct(translation_unit unit, struct file file) {

    raise_indents(unit.list, file, 0);
    turn_indents_into_blocks(unit.list, file, 0);
    ///remove_whitespace_in_expressions(unit.list, file, 0);
    
    /// TODO: code:    remove_empty_statements_in_blocks(unit.list, file, 0);
    ///TODO: delete "clean(block body)" in analysis phase, after coding remove_empty_statements_in_blocks().

    if (debug) {
        std::cout << "------------------- corrector: -------------------------\n";
        print_translation_unit(unit, file);
    }

    return unit;
}
