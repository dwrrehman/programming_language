//
//  analysis.cpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "analysis.hpp"
#include "parser.hpp"
#include "nodes.hpp"
#include "lists.hpp"

#include "debug.hpp"

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <algorithm>


/*   known bug:

        parse _type end

        solution: {ERROR}

TODO:



        make a quick and simple expression parser, which prints it on one line, very simply, using parens for subexpressions.

 */

/// Global builtin types. these are fundemental to the language:
// _type is also just nullptr.

expression unit_type = {};
expression type_type = {{{"_type", false}}};
expression none_type = {{{"_none", false}}};
expression infered_type = {{{"_infered", false}}};

expression i32_type = {{{"_i32", false}}};
expression exit_abstraction = {
    {
        {"_exit", false},
        {{{}, &i32_type}}
    }, &unit_type};


/////////////// TESTING SIGNATURES //////////////////////////


expression int_type = {
    {
        {"int", false}
    }, &unit_type};

expression int0_literal = {
    {
        {"0", false}
    }, &i32_type};

expression dog_type = {
    {
        {"dog", false}
    }, &int_type};

expression x_type = {
    {
        {"x", false}
    }, &dog_type};

expression print_type = {
    {
        {"print", false},
        {{{}, &int_type}},
    }, &int_type};

expression is_good_type = {
    {
        {{{}, &dog_type}},
        {"is", false},
        {"good", false},
    }, &int_type};

expression unit_to_int_type = {
    {
        {{{}, &unit_type}}
    }, &int_type};

expression int_to_unit_type = {
    {
        {{{}, &int_type}}
    }, &unit_type};


////////////////////////////////////////////////////////////////////



std::vector<expression> builtins =  {
    unit_type, none_type, infered_type, i32_type, exit_abstraction,

    // TESTING:
    int_type, dog_type, print_type, int_to_unit_type,
    unit_to_int_type, is_good_type, x_type, int0_literal,
};




bool expressions_match(expression first, expression second);
expression csr(const std::vector<expression> list, const expression given, const size_t depth, const size_t max_depth, size_t& pointer, struct expression*& type);


bool symbols_match(symbol first, symbol second) {
    if (first.type == symbol_type::subexpression && second.type == symbol_type::subexpression)
        return expressions_match(first.subexpression, first.subexpression);
    else if (first.type == symbol_type::identifier && second.type == symbol_type::identifier
             && first.identifier.name.value == second.identifier.name.value) return true;
    else return false;
}

bool expressions_match(expression first, expression second) {
    if (first.symbols.size() != second.symbols.size()) return false;
    for (size_t i = 0; i < first.symbols.size(); i++) {
        if (!symbols_match(first.symbols[i], second.symbols[i])) return false;
    }
    if (first.erroneous || second.erroneous) return false;
    if ((!first.type && !second.type) || ((first.type && second.type) && expressions_match(*first.type, *second.type))) return true;
    else return false;
}


void prune_extraneous_subexpressions(expression& given) {
    while (given.symbols.size() == 1
           && given.symbols[0].type == symbol_type::subexpression
           && given.symbols[0].subexpression.symbols.size()) {
        auto save = given.symbols[0].subexpression.symbols;
        given.symbols = save;
    }
    for (auto& symbol : given.symbols)
        if (symbol.type == symbol_type::subexpression) prune_extraneous_subexpressions(symbol.subexpression);
}





























expression csr(const std::vector<expression> list, const expression given, const size_t depth, const size_t max_depth, size_t& pointer, struct expression*& type) {
    
    if (depth > max_depth) return {true};
    if (type && expressions_match(*type, none_type)) return {true};
    if (given.symbols.empty() || (given.symbols.size() == 1
                                   && given.symbols[0].type == symbol_type::subexpression
                                   && given.symbols[0].subexpression.symbols.empty())) {
        if (given.symbols.size() == 1
            && given.symbols[0].type == symbol_type::subexpression
            && given.symbols[0].subexpression.symbols.empty()) pointer++;
        if (type && expressions_match(*type, infered_type)) type = &unit_type;
        if (!type || expressions_match(*type, unit_type)) return {{}, &unit_type};
        else return {true};
    }
    const size_t saved = pointer;
    for (auto signature : list) {
        if (type && !expressions_match(*type, infered_type) && (!signature.type || !expressions_match(*type, *signature.type))) continue;
        expression solution = {};
        pointer = saved;
        bool failed = false;
        for (auto element : signature.symbols) {
            if (pointer >= given.symbols.size()) { failed = true; break; }
            if (element.type == symbol_type::subexpression) {
                auto subexpression = csr(list, given, depth + 1, max_depth, pointer, element.subexpression.type);
                if (subexpression.erroneous) {
                    if (given.symbols[pointer].type == symbol_type::subexpression) {
                        size_t local_pointer = 0, current_depth = 0;
                        expression subexpression = {};
                        while (current_depth <= max_expression_depth) {
                            local_pointer = 0;
                            subexpression = csr(list, given.symbols[pointer].subexpression, 0, current_depth, local_pointer, element.subexpression.type);
                            if (subexpression.erroneous || local_pointer < given.symbols[pointer].subexpression.symbols.size()) {
                                current_depth++;
                            } else break;
                        }
                        if (subexpression.erroneous || local_pointer < given.symbols[pointer].subexpression.symbols.size()) { failed = true; break; }
                        solution.symbols.push_back({subexpression});
                        pointer++; continue;
                    } else { failed = true; break; }
                } solution.symbols.push_back({subexpression});
            } else if (element.identifier.name.value == given.symbols[pointer].identifier.name.value
                       && given.symbols[pointer].type == symbol_type::identifier && element.type == symbol_type::identifier) {
                solution.symbols.push_back(element);
                pointer++;
            } else { failed = true; break; }
        } if (!failed) {
            if (type && expressions_match(*type, infered_type)) type = signature.type;
            solution.type = signature.type;
            return solution;
        }
    } return {true};
}

















void test_csr(translation_unit unit, struct file file) {

    expression int_type = {
        {
            {"int", false}
        }, &unit_type};

    expression dog_type = {
        {
            {"dog", false}
        }, &int_type};

    expression x_type = {
        {
            {"x", false}
        }, &dog_type};

    expression print_type = {
        {
            {"print", false},
            {{{}, &int_type}},
        }, &int_type};

    expression is_good_type = {
        {
            {{{}, &dog_type}},
            {"is", false},
            {"good", false},
        }, &int_type};

    expression unit_to_int_type = {
        {
            {{{}, &unit_type}}
        }, &int_type};

    std::vector<expression> signatures = {none_type, infered_type, int_type, dog_type, print_type, unit_to_int_type, x_type, is_good_type};

    std::sort(signatures.begin(), signatures.end(), [](auto a, auto b) { return a.symbols.size() > b.symbols.size(); });

    expression given = {};
    if (unit.list.expressions.size()) {
        given = unit.list.expressions[0];
    }

    std::cout << "parsing: ";
    print_expression(given, 0);


//////////////////// how to use csr ////////////////////////
    prune_extraneous_subexpressions(given);
    
    size_t pointer = 0;
    expression solution = {};
    expression* type = &infered_type;
    size_t max_depth = 0;

    while (max_depth <= max_expression_depth) {
        std::cout << "trying depth = " << max_depth << std::endl; // debug
        pointer = 0;
        type = &infered_type;
        solution = csr(signatures, given, 0, max_depth, pointer, type);
        if (solution.erroneous || pointer < given.symbols.size()) {
            max_depth++;
        }
        else break;
    }
////////////////////////////////////////////////////////////////

    
    std::cout << "\nsolution: ";
    print_expression(solution, 0);
    std::cout << "\n";

    std::cout << "it has type = ";
    if (type) print_expression(*type, 0);
    else std::cout << "{_type}";
    std::cout << "\n";

    if (pointer < given.symbols.size()) {
        std::cout << "(but its erroenous)...\n";
    }
}




expression resolve(std::vector<expression> list, expression given, expression solution_type) {

    std::sort(list.begin(), list.end(), [](auto a, auto b) { return a.symbols.size() > b.symbols.size(); });
    prune_extraneous_subexpressions(given);

    std::cout << "printing given list:\n";
    for (auto l : list) {
        print_expression(l, 0);
        std::cout << "\n\n";
    }

    std::cout << "finished.\n";

    std::cout << "now printing givne expression:\n";

    print_expression(given, 0);


    std::cout << "\n\nfinsihed.\n";


    size_t pointer = 0, max_depth = 0;
    expression solution = {};
    auto sol_t_cop = solution_type;
    while (max_depth <= max_expression_depth) {
        pointer = 0;
        solution.type = &sol_t_cop;
        solution = csr(list, given, 0, max_depth, pointer, solution.type);
        if (solution.erroneous || pointer < given.symbols.size()) { max_depth++; }
        else break;
    }
    if (pointer < given.symbols.size()) solution.erroneous = true;
    return solution;
}


abstraction_definition adp(expression given) {

    return {};
}


void wrap_into_main(translation_unit& unit) {
    auto main_body = unit.list;
    expression main_call_signature = {{{"_main", false}}};
    expression main_return_type = {{{"_i32", false}}};
    abstraction_definition main_abstraction = {main_call_signature, main_return_type, {main_body}};
    symbol main_symbol = {main_abstraction};
    expression top_level_expression = {{main_symbol}};
    unit.list.expressions.clear();
    unit.list.expressions.push_back(top_level_expression);
}

void append_test_sigs(std::vector<expression>& table) {


}

translation_unit analyze(translation_unit unit, struct file file) {

    wrap_into_main(unit);
    std::vector<std::vector<expression>> tables = {builtins};
    auto& main = unit.list.expressions[0].symbols[0].abstraction;
    auto& body = main.body.list.expressions;

    bool error = false;

    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size(); i++) {
            auto solution = resolve(tables.back(), body[i], unit_type);
            if (solution.erroneous) {
                std::cout << "n3zqx2l: csr: fake error: Could not parse expression!\n"; // TODO: print an error of some kind!
                error = true;
                continue;
            }
            parsed_body.push_back(solution);
        }
        main.body.list.expressions = parsed_body;
    } else {
        main.return_type = unit_type;
    }

    if (debug) {
        std::cout << "----------------- analyzer ---------------------\n";
        print_translation_unit(unit, file);
    }

    if (error) {
        std::cout << "\n\n\tCSR ERROR\n\n\n\n";
    } else {
        std::cout << "\n\n\tsuccess.\n\n\n";
    }

    return unit;
}


// note for future self:

    // simply use a static global, which is in common between all calls to analyze().
    // when we need to make sure that there is only one file which contains top level statements,
    // we simply look at this global variable, called "found_main". if true, and we want to makr this current file as main, error.
