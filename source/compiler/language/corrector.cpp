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



/**

nostril do file.n


use .io
print "hello, world!"



 -------------------- STAGES ------------------


parsing - corrector:

    stage 1:   indent raising (IR) phase

    stage 1.1  indent to block transformations?

    stage 2:   expression to abstraction (EA) correction phase

    stage 3:   expression to variable (EV) correction phase


analysis:

    stage 4:    scope and visibility analysis (SVA) phase

    stage 5:    type inference and checking (TIC) phase

    stage 6:    namespace signature subsitution (NSS) phase

    stage 7:    call signature resolution (CSR) phase

    stage 8:    numeric value subsitution (NVS) phase


code generation:

    ...



optimization:

    ...

 stage ...:    compiletime abstraction evaluation (CAE) phase

linking:

    ...






------- HOW TO DO EXP_TO_ABS (ETA stage) -------

 0. check if its a function call.
 1. check if it takes the form of a prototype.
 2. check if it takes the form of a definition signature.
 3. check if it takes the form of a call signature
 4. check if it takes the form of a type signature.
 5. if not any of these, throw an error: "unresolved expression: my func () hello () from space "






 note: everything is assumed to be a function call at first.

characteristics of each:


    0. a function call will never have any colons. thats the best we can do rright now.

    1. a prototype will always have a colon, at least for the signature type.
    1. it may also have colons in the call signature portion.

    2. it will always either have a block, or





 */


void turn_indents_into_blocks(expression_list& list, struct file file, const size_t level);

void add_block_to_list(block& block, struct file file, size_t level, expression_list& new_list) {
    turn_indents_into_blocks(block.list, file, level + 1);
    symbol s {};
    s.type = symbol_type::block;
    s.block = block;
    class expression e {};
    e.error = false;
    e.symbols.push_back(s);
    if (new_list.expressions.empty()) new_list.expressions.push_back(e);
    else new_list.expressions.back().symbols.push_back(s);
}

/// ---------------- stage 2: TIB ----------------------------

void turn_indents_into_blocks(expression_list& list, struct file file, const size_t level) {

    expression_list new_list {};
    block block {};
    bool inside_block = false;

    for (auto& expression : list.expressions) {
        if (expression.indent_level > level && !inside_block) {
            block.list.expressions.push_back(expression);
            inside_block = true;
        } else if (expression.indent_level > level && inside_block) {
            block.list.expressions.push_back(expression);
        } else {
            if (block.list.expressions.size())
                add_block_to_list(block, file, level, new_list);
            new_list.expressions.push_back(expression);
            inside_block = false;
            block.list.expressions.clear();
        }
    }
    if (block.list.expressions.size())
        add_block_to_list(block, file, level, new_list);
    list = new_list;
}









/// ------------------ stage 1: IR --------------------------

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

    std::cout << "------------------- corrector: -------------------------\n";


    raise_indents(unit.list, file, 0);
    turn_indents_into_blocks(unit.list, file, 0);

    print_translation_unit(unit, file); // debug

    return {};
}
