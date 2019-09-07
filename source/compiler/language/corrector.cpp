//
//  corrector.cpp
//  language
//
//  Created by Daniel Rehman on 1903192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "corrector.hpp"

#include "lists.hpp"
#include "arguments.hpp"
#include "helpers.hpp"
#include "debug.hpp"

#include <iostream>

/*
        known bug:

            when we give a indent level of 2, it only finds 1 block from it...?

 */

inline static bool is_whitespace(const symbol& e) { return e.type == symbol_type::indent or e.type == symbol_type::newline; }

void remove_whitespace_in_expressions(expression_list& list, file file, size_t depth) {    
    for (auto& expression : list.list) {
        auto& s = expression.symbols;
        if (std::find_if(s.begin(), s.end(), is_whitespace) != s.end()) 
            s.erase(std::remove_if(s.begin(), s.end(), is_whitespace));        
        for (auto& symbol : s) 
            if (subexpression(symbol)) remove_whitespace_in_expressions(symbol.expressions, file, depth);                      
    }
}

void turn_indents_into_blocks(expression_list& list, file file, const size_t level);

static void push_block_onto_list(expression_list& list, file file, const size_t level, expression_list& new_list) {
    if (list.list.size()) {
        turn_indents_into_blocks(list, file, level + 1);
        if (new_list.list.empty()) new_list.list.push_back({{{list}}});
        else new_list.list.back().symbols.push_back({list});
    }
}

void turn_indents_into_blocks(expression_list& given, file file, const size_t level) {

    expression_list new_list {}, block {};
    for (auto& expression : given.list) {
        if (expression.symbols.empty()) continue;        
        if (expression.indent_level > level) block.list.push_back(expression);
        else {
            push_block_onto_list(block, file, level, new_list);
            new_list.list.push_back(expression);
            block.list.clear();
        }
    }
    push_block_onto_list(block, file, level, new_list);
    given = new_list;
}

void raise(size_t& value, const size_t minimum) {
    if (value < minimum) value = minimum;
}

void raise_indents(expression_list& list, file file, const size_t level) {
    for (auto& expression : list.list) {
        raise(expression.indent_level, level);
        for (auto& symbol : expression.symbols)
            if (symbol.type == symbol_type::subexpression)
                raise_indents(symbol.expressions, file, level + 1);
    }
}

expression_list correct(expression_list unit, file file) {
    
    raise_indents(unit, file, 0);
    turn_indents_into_blocks(unit, file, 0);
    
    remove_whitespace_in_expressions(unit, file, 0);
    
    if (debug) {
        std::cout << "------------------- corrector: -------------------------\n";
        print_translation_unit(unit, file);
    }
    return unit;
}




/// TODO: code:    remove_empty_statements_in_blocks(unit.list, file, 0);           // previously named "clean(body)"
/// then...
///TODO: delete "clean(block body)" in analysis phase, after coding remove_empty_statements_in_blocks().
