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




/*

 known bug:

 parse _type end

 solution: {ERROR}

 */


/// Global builtin types. these are fundemental to the language.

// _type is nullptr
expression unit_type = {};
expression nothing_type = {{{"_none", false}}};
expression infered_type = {{{"_infered", false}}};



bool expressions_match(expression first, expression second);

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
    if ((!first.type && !second.type) || expressions_match(*first.type, *second.type)) return true;
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
    if (type && expressions_match(*type, nothing_type)) return {true};
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


/*
    signature int_type = {
        {
            {"int", {}, false}
        }, &unit_type, false};

    signature dog_type = {
        {
            {"dog", {}, false}
        }, &int_type, false};

    signature print_type = {
        {
            {"print", {}, false},
            {"", {{}, &int_type, false}, true}
        }, &int_type, false};

    signature unit_to_int_type = {
        {
            {"", {{}, &unit_type, false}, true}
        }, &int_type, false};
    */
    std::vector<expression> signatures = {nothing_type, infered_type, /*int_type, print_type, dog_type, unit_to_int_type*/};

    std::sort(signatures.begin(), signatures.end(), [](auto a, auto b) { return a.symbols.size() > b.symbols.size(); });

    expression given = {};
    if (unit.list.expressions.size()) {
        given = unit.list.expressions[0];
    }

    std::cout << "parsing: ";
    print_expression(given, 0);

    prune_extraneous_subexpressions(given);
    
    size_t pointer = 0;
    expression solution = {};
    expression* type = &infered_type;
    size_t max_depth = 0;

    while (max_depth <= max_expression_depth) {
        std::cout << "trying depth = " << max_depth << std::endl;
        pointer = 0;
        type = &infered_type;
        solution = csr(signatures, given, 0, max_depth, pointer, type);
        if (solution.erroneous || pointer < given.symbols.size()) {
            max_depth++;
        }
        else break;
    }

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













translation_unit analyze(translation_unit unit, struct file file) {

    std::cout << "----------------- analyzer ---------------------\n";
    print_translation_unit(unit, file);


    test_csr(unit, file);


    return {};
}




/*
signature csr(const std::vector<signature> list, const signature given, const size_t depth, const size_t max_depth, size_t& pointer, struct signature*& type) {

    if (depth > max_depth) return {{}, nullptr, true};
    if (type && signatures_match(*type, nothing_type)) return {{}, nullptr, true};
    if (given.elements.empty() || (given.elements.size() == 1
                                   && given.elements[0].is_parameter
                                   && given.elements[0].children.elements.empty())) {
        if (given.elements.size() == 1
            && given.elements[0].is_parameter
            && given.elements[0].children.elements.empty()) pointer++;
        if (type && signatures_match(*type, infered_type)) type = &unit_type;
        if (!type || signatures_match(*type, unit_type)) return {{}, &unit_type};
        else return {{}, nullptr, true};
    }
    const size_t saved = pointer;
    for (auto signature : list) {
        if (type && !signatures_match(*type, infered_type) && (!signature.type || !signatures_match(*type, *signature.type))) continue;
        struct signature solution = {};
        pointer = saved;
        bool failed = false;
        for (auto element : signature.elements) {
            if (pointer >= given.elements.size()) { failed = true; break; }
            if (element.is_parameter) {
                auto subexpression = csr(list, given, depth + 1, max_depth, pointer, element.children.type);
                if (subexpression.erroneous) {
                    if (given.elements[pointer].is_parameter) {
                        size_t local_pointer = 0, current_depth = 0;
                        struct signature subexpression = {};
                        while (current_depth <= max_expression_depth) {
                            local_pointer = 0;
                            subexpression = csr(list, given.elements[pointer].children, 0, current_depth, local_pointer, element.children.type);
                            if (subexpression.erroneous || local_pointer < given.elements[pointer].children.elements.size()) {
                                current_depth++;
                            } else break;
                        }
                        if (subexpression.erroneous || local_pointer < given.elements[pointer].children.elements.size()) { failed = true; break; }
                        solution.elements.push_back({"", subexpression, true});
                        pointer++; continue;
                    } else { failed = true; break; }
                } solution.elements.push_back({"", subexpression, true});
            } else if (element.name == given.elements[pointer].name) {
                solution.elements.push_back(element);
                pointer++;
            } else { failed = true; break; }
        } if (!failed) {
            if (type && signatures_match(*type, infered_type)) type = signature.type;
            solution.type = signature.type;
            return solution;
        }
    } return {{}, nullptr, true};
}
*/
