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



 -------------------- STAGES ------------------

parsing - corrector:

    stage 1:   indent level raising (ILR) indent correction phase

    stage 2    turn indent to block transformations (TIB) indent correction phase

    stage 3:   expression to abstraction definition (ETA) correction phase

    stage 4:   expression to variable definition (ETV) correction phase



 possible stage 5:      turn top level abstraction call expression(s (of form: subexpr, stuff, block) into abstraction definitions. (they cant possibly be a fucntion call, becuase its the top level.
        aka, top level expression to abstraction definition:      (TEA)




analysis:

    stage 5:    scope and visibility analysis (SVA) phase

    stage 6:    namespace signature subsitution (NSS) phase

    stage 7:    type inference and checking (TIC) phase

    stage 8:    call signature resolution (CSR) phase

    stage 9:    numeric value subsitution (NVS) phase


 post analysis:

    stage 10:   wrapper type expansion (WTE) phase

    stage 11:   signature ABI transformation (SAT) phase

code generation:

    CGN



optimization:

    OPT



 stage ...:    compiletime abstraction evaluation (CAE) phase

linking:

    LNK







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









 what we need to allow for is that:

        x: (a b) c = (x: a y: b) c : _runtime {


 the algorithm of spotting abstraction definitions is as so:


 note: we always need to allow for variables
 to be on their own line, alwaus.

 heres a note though:


 we only need to mak abstractions alloweed to be in
 expressions (in the middleof expressions)


 variables cant be in the middle of expressions,
 they are always on their own line.

 another reason we need to allow for abstraction definitions
 (but not prototypes) to be anywwehere in a function definition,
 is for when you want to pass a user defined
 lambda into a another function call.


 heres another idea:


 the algorithm for this is as follows:


 walk a expression:


 if the current symbol is a subexpression,

 then walk untill you find the first block.
 then, everything from the subexpression and the block mightttt be
 a abstraction definition, IF AND ONLY IF there is a colon between
 the two end points.



 heres a question:


 do we need variables as their own statement, always?


 i think so, actually. this is because of the nature of the variable definition:


 its always an expression, followed by a colon, followed by a ;

 however there are some restrictions on what you can have on the left hand side: (ie, the identifier side)

 these restrictions are:

 no strings of any kinds:
 - no doc strings, no string literals, no llvm strings, and no character strings.

 this is mandatory, i think.

 although... why? why not have those things...?  we will see, i guess.

 you cannot have parenthesis on this side. if you do, then you are actually defining an abstraction.


 however, there are some interesting cavieats.

 if you do something like this:      (this is allowable:)


 (f): (c) = (my func) {
    ; body here
 }

 note: that "f" here, is a abstraction, but is being defined inside of a variable definition.



 this is because a variable can be "of type \"abstraction\""



 however, because a abstraction can take on multiple forms:


 (x) c : asdf {        ; very easy, this is probably what we will implement first.
    print hi
 }

 (x): asdf
    print hi           ; also very easy.


 (x) c {                ; this one is impossible, until after CSR.

 }

(x)                            ; this one is pretty difficult, but not after csr.
    print hi



 IMPOSSIBLE:

 (x) print hi                    ; this is actually impossible. like actually.

 (x): print hi                  ; this is impossible, actually. for functions, the block cannot be implied, because its right next to an expression, the type signature.

 (x) c : asdf print hi          ; this is impossible for the same reason


 */

/// Helpers:

static bool is_colon(const symbol &symbol) {
    return symbol.type == symbol_type::identifier &&
    symbol.identifier.name.type == token_type::operator_ &&
    symbol.identifier.name.value == ":";
}

bool contains_a_colon(expression expression) {
    for (auto s : expression.symbols)
        if (is_colon(s)) return true;
    return false;
}

/// ------------------- stage 4: ETV --------------------------


void find_variable_definitions(expression_list& list, struct file file) {

    for (auto& expression : list.expressions) {

        for (auto& symbol :expression.symbols) {
            if (symbol.type == symbol_type::block) {
                find_variable_definitions(symbol.block.list, file);
            }
        }

        if (expression.symbols.size() && contains_a_colon(expression)) {
            variable_definition variable = {};

            size_t i = 0;
            for (; i < expression.symbols.size(); i++) {
                if (is_colon(expression.symbols[i])) break;
                variable.name.symbols.push_back(expression.symbols[i]);
            }
            i++;
            for (; i < expression.symbols.size(); i++) {
                variable.type.symbols.push_back(expression.symbols[i]);
            }

            expression.symbols.clear();
            symbol s {};
            s.type = symbol_type::variable_definition;
            s.variable = variable;
            expression.symbols.push_back(s);
        }
    }
}


/// ---------------- stage 3: ETA ----------------------------

void find_abstraction_definitions(expression_list& list, struct file file) {

    for (auto& expression : list.expressions) {
        for (size_t i = 0; i < expression.symbols.size(); i++) {
            if (expression.symbols[i].type == symbol_type::block) find_abstraction_definitions(expression.symbols[i].block.list, file);
            if (expression.symbols[i].type == symbol_type::subexpression) { // REDO THIS: use the STL algorithms for the finding/transforming.

                bool is_abstraction = true;
                abstraction_definition abstraction = {};
                abstraction.call = expression.symbols[i].subexpression;
                const size_t start = i++;

                while (!is_colon(expression.symbols[i])) {
                    if (i >= expression.symbols.size() || expression.symbols[i].type == symbol_type::block) { is_abstraction = false; break; }
                    abstraction.return_type.symbols.push_back(expression.symbols[i++]);
                }
                if (!is_abstraction) { i = start; continue; }
                i++;
                while (expression.symbols[i].type != symbol_type::block) {
                    if (i >= expression.symbols.size()) { is_abstraction = false; break; }
                    abstraction.signature_type.symbols.push_back(expression.symbols[i++]);
                }
                if (!is_abstraction) { i = start; continue; }

                abstraction.body = expression.symbols[i++].block;
                find_abstraction_definitions(abstraction.body.list, file);
                const auto end = i;
                expression.symbols.erase(expression.symbols.begin() + start, expression.symbols.begin() + end);

                symbol s {};
                s.type = symbol_type::abstraction_definition;
                s.abstraction = abstraction;
                expression.symbols.insert(expression.symbols.begin() + start, s);
            }
        }
    }
}


/// ---------------- stage 2: TIB ----------------------------

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

void turn_indents_into_blocks(expression_list& list, struct file file, const size_t level) {

    expression_list new_list {};
    block block {};
    bool inside_block = false;

    for (auto& expression : list.expressions) {
        if (expression.symbols.empty()) continue;

        if (expression.indent_level > level) {
            block.list.expressions.push_back(expression);
            if (!inside_block) inside_block = true;
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

/// ------------------ stage 1: ILR --------------------------

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


// the main corrector function:

translation_unit correct(translation_unit unit, struct file file) {

    std::cout << "------------------- corrector: -------------------------\n";

    raise_indents(unit.list, file, 0);
    turn_indents_into_blocks(unit.list, file, 0);
    find_abstraction_definitions(unit.list, file);
    find_variable_definitions(unit.list, file);

    print_translation_unit(unit, file); // debug

    return {};
}
