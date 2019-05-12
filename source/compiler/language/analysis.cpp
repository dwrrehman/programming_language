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


/// Global builtin types. these are fundemental to the language:
expression type_type = {{{"_type", false}}};
expression none_type = {{{"_none", false}}, &type_type};
expression unit_type = {{}, &type_type};
expression infered_type = {{{"_infered", false}}};

expression i32_type = {{{"_i32", false}}, &type_type};
expression exit_abstraction = {
    {
        {"_exit", false},
        {{{}, &i32_type}}
    }, &unit_type};

/////////////// TESTING SIGNATURES //////////////////////////

expression int_type = {
    {
        {"int", false},
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

expression abs_type = {
    {
        {{{unit_type}}},
    }, &type_type};

expression abs_to_unit_type = {
    {
        {{{}, &abs_type}},
    }, &unit_type};

expression print_abs_type = {
    {
        {"print", false},
        {"abs", false},
        {{{}, &abs_type}},
    }, &unit_type};

expression is_good_type = {
    {
        {{{}, &dog_type}},
        {"is", false},
        {"good", false},
    }, &int_type};

////////////////////////////////////////////////////////////////////

std::vector<expression> builtins =  {
    type_type, unit_type, none_type, infered_type, i32_type, exit_abstraction,

    // TESTING:
    int_type, dog_type, print_type,
    is_good_type, x_type, int0_literal,

    print_abs_type, abs_type, abs_to_unit_type,
};

bool expressions_match(expression first, expression second);
expression csr(std::vector<std::vector<expression>>& stack, const expression given, const size_t depth, const size_t max_depth, size_t& pointer, struct expression*& expected, bool can_define_new_signature);
bool adp(abstraction_definition& given, std::vector<std::vector<expression>>& stack);
expression resolve(std::vector<std::vector<expression>>& stack, expression given, expression& solution_type);

bool symbols_match(symbol first, symbol second) {
    if (first.type == symbol_type::subexpression and second.type == symbol_type::subexpression and expressions_match(first.subexpression, second.subexpression)) return true;
    else if (first.type == symbol_type::identifier and second.type == symbol_type::identifier and first.identifier.name.value == second.identifier.name.value) return true;
    else return false;
}

bool expressions_match(expression first, expression second) {
    if (first.symbols.size() != second.symbols.size()) return false;
    for (size_t i = 0; i < first.symbols.size(); i++) {
        if (not symbols_match(first.symbols[i], second.symbols[i])) return false;
    }
    if (first.erroneous or second.erroneous) return false;
    if (!first.type and !second.type) return true;
    else if (first.type and second.type and expressions_match(*first.type, *second.type)) return true;
    else return false;
}

void prune_extraneous_subexpressions(expression& given) {
    while (given.symbols.size() == 1
           and given.symbols[0].type == symbol_type::subexpression
           and given.symbols[0].subexpression.symbols.size()) {
        auto save = given.symbols[0].subexpression.symbols;
        given.symbols = save;
    }
    for (auto& symbol : given.symbols)
        if (symbol.type == symbol_type::subexpression) prune_extraneous_subexpressions(symbol.subexpression);
}

std::vector<symbol> filter_subexpressions(expression call_signature) {
    std::vector<symbol> result = {};
    for (auto element : call_signature.symbols) {
        if (element.type == symbol_type::subexpression) {
            result.push_back(element);
        }
    }
    return result;
}

expression* generate_abstraction_type_for(abstraction_definition def) {
    auto type = new expression();        ///TODO: free this at some point.
    type->was_allocated = true;
    auto parameter_list = filter_subexpressions(def.call_signature);
    for (auto parameter : parameter_list) {
        expression t = type_type;
        if (parameter.subexpression.type) t = *parameter.subexpression.type;
        type->symbols.push_back(t);
    }
    type->symbols.push_back({def.return_type});
    type->type = &type_type;
    return type;
}

void clean(block& body) {
    block result = {};
    for (auto expression : body.list.expressions) {
        if (not expression.symbols.empty()) result.list.expressions.push_back(expression);
    } body = result;
}

static void parse_abstraction_body(bool &error, abstraction_definition &given, std::vector<std::vector<expression>> &stack) {
    stack.back().push_back(given.call_signature); // allow for recursion.
    std::cout << "pushed: ";
    print_expression_line(given.call_signature);
    std::cout << "\n";

    auto& body = given.body.list.expressions;
    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size() - 1; i++) {
            auto type = unit_type;
            auto solution = resolve(stack, body[i], type);
            if (solution.erroneous) {
                std::cout << "n3zqx2l: adp-csr: fake error: Could not parse expression!\n"; // TODO: print an error (IN CSR!) of some kind!

                auto type = infered_type;
                auto actual = resolve(stack, body[i], type);

                std::cout << "the actual type was: \n";
                print_expression_line(type);
                std::cout << "\n\n";

                std::cout << "was expecting: \n";
                print_expression_line(unit_type);
                std::cout << "\n\n";

                error = true;
                continue;
            }
            parsed_body.push_back(solution);
        }
        auto solution = resolve(stack, body[body.size() - 1], given.return_type);
        if (solution.erroneous) {
            std::cout << "n3zqx2l: adp-csr: fake error: Could not parse return expression!\n"; // TODO: print an error (IN CSR!) of some kind!

            auto type = infered_type;
            auto actual = resolve(stack, body[body.size() - 1], type);

            std::cout << "the actual type was: \n";
            print_expression_line(type);
            std::cout << "\n\n";

            std::cout << "was expecting: \n";
            print_expression_line(given.return_type);
            std::cout << "\n\n";

            error = true;
        }
        parsed_body.push_back(solution);
        given.body.list.expressions = parsed_body;
    } else if (expressions_match(given.return_type, infered_type)) {
        given.return_type = unit_type;
    }
}

static void parse_return_type(abstraction_definition &given, std::vector<std::vector<expression> > &stack, bool& error) {
    ///TODO: this function needs to evaluate its return type, at compiletime.
    if (given.return_type.symbols.size()) {
        auto type = infered_type;
        given.return_type = resolve(stack, given.return_type, type);
        if (given.return_type.erroneous) {
            std::cout << "n3zqx2l: adp-csr: fake error: Could not parse return type.\n"; // TODO: print an error (IN CSR!) of some kind!
            error = true;
        }
        if (given.return_type.type and expressions_match(*given.return_type.type, unit_type) and expressions_match(given.return_type, unit_type)) given.return_type = unit_type;
        else if (given.return_type.type and expressions_match(*given.return_type.type, none_type)) given.return_type = none_type;
    } else given.return_type = infered_type;
    given.call_signature.type = &given.return_type;
}

static void parse_signature(abstraction_definition &given, std::vector<std::vector<expression>>& stack, bool& error) {
    expression result = {};
    auto call = given.call_signature.symbols;
    for (size_t i = 0; i < call.size(); i++) {
        if (call[i].type == symbol_type::subexpression) {

            auto sub = call[i].subexpression;
            abstraction_definition definition = {};
            size_t pointer = 0;

            if (sub.symbols.empty()) {
                std::cout << "n3zqx2l: fake error: signature subexpression must not be empty.\n";
                error = true;
                continue;

            } else if (sub.symbols[pointer].type == symbol_type::subexpression) {
                definition.call_signature = sub.symbols[pointer++].subexpression;
                while (pointer < sub.symbols.size()) {
                    definition.return_type.symbols.push_back(sub.symbols[pointer++]);
                }
                parse_signature(definition, stack, error);
                parse_return_type(definition, stack, error);
                auto parameter_type = generate_abstraction_type_for(definition);
                expression parameter = {definition.call_signature.symbols, parameter_type};
                result.symbols.push_back({parameter});
                stack.back().push_back(parameter);
            } else {
                expression parameter = {sub.symbols, &infered_type};
                result.symbols.push_back({parameter});
                stack.back().push_back(parameter);
            }
        } else if (call[i].type == symbol_type::identifier) {
            result.symbols.push_back(call[i]);

        } else { //TODO: add additional cases for the other symbol types. (like strings, etc.)
            std::cout << "error, unexpected " <<  convert_symbol_type(call[i].type) << "...\n";
            std::cout << "ignoring...\n";
            error = true;
        }
    }
    std::cout << "parsing call signature...\n";
    std::cout << "was: ";
    print_expression_line(given.call_signature);
    std::cout << "\n";

    given.call_signature = result;

    std::cout << "now its: ";
    print_expression_line(given.call_signature);
    std::cout << "\n";
}

bool adp(abstraction_definition& given, std::vector<std::vector<expression>>& stack) {
    clean(given.body); // move me into the corrector code.
    bool error = false;
    stack.push_back(stack.back());
    parse_signature(given, stack, error);
    parse_return_type(given, stack, error);
    parse_abstraction_body(error, given, stack);
    stack.pop_back();
    return error;
}

bool contains_a_block_starting_from(size_t begin, std::vector<symbol> list) {
    for (; begin < list.size(); begin++)
        if (list[begin].type == symbol_type::block) return true;
    return false;
}

expression csr(std::vector<std::vector<expression>>& stack, const expression given, const size_t depth, const size_t max_depth, size_t& pointer, struct expression*& expected, bool can_define_new_signature) {
    const auto list = stack.back();
    if (depth > max_depth) return {true};
    if (!expected) return {true};
    if (expressions_match(*expected, none_type)) return {true};
    if (given.symbols.empty() or (given.symbols.size() == 1
                                  and given.symbols[0].type == symbol_type::subexpression
                                  and given.symbols[0].subexpression.symbols.empty())) {
        if (given.symbols.size() == 1
            and given.symbols[0].type == symbol_type::subexpression
            and given.symbols[0].subexpression.symbols.empty()) pointer++;
        if (expressions_match(*expected, infered_type)) expected = &unit_type;
        if (expressions_match(*expected, unit_type)) return {{}, &unit_type};
        else return {true};
    }
    const size_t saved = pointer;
    for (auto& signature : list) {
        if (not expressions_match(*expected, infered_type) and signature.type and not expressions_match(*expected, *signature.type)) continue;
        expression solution = {};
        pointer = saved;
        bool failed = false;
        for (auto& element : signature.symbols) {
            if (pointer >= given.symbols.size()) { failed = true; break; }
            if (element.type == symbol_type::subexpression) {
                auto TEMP = element.subexpression.type;
                auto subexpression = csr(stack, given, depth + 1, max_depth, pointer, TEMP, false);
                if (subexpression.erroneous) {
                    pointer = saved;
                    if (given.symbols[pointer].type == symbol_type::subexpression) {
                        size_t local_pointer = 0, current_depth = 0;
                        expression subexpression = {};
                        while (current_depth <= max_expression_depth) {
                            local_pointer = 0;
                            auto TEMP = element.subexpression.type;
                            subexpression = csr(stack, given.symbols[pointer].subexpression, 0, current_depth, local_pointer, TEMP, false);
                            if (subexpression.erroneous or local_pointer < given.symbols[pointer].subexpression.symbols.size()) {
                                current_depth++;
                            } else break;
                        }
                        if (subexpression.erroneous || local_pointer < given.symbols[pointer].subexpression.symbols.size()) { failed = true; break; }
                        solution.symbols.push_back({subexpression});
                        pointer++; continue;
                    } else { failed = true; break; }
                } solution.symbols.push_back({subexpression});
            } else if (element.identifier.name.value == given.symbols[pointer].identifier.name.value
                       and given.symbols[pointer].type == symbol_type::identifier
                       and element.type == symbol_type::identifier) {
                solution.symbols.push_back(element);
                pointer++;
            } else { failed = true; break; }
        } if (!failed) {
            if (expressions_match(*expected, infered_type)) expected = signature.type;
            if (signature.type) solution.type = signature.type; else solution.type = &type_type;
            return solution;
        }
    }
    if (given.symbols[pointer].type == symbol_type::subexpression && contains_a_block_starting_from(pointer + 1, given.symbols)) {
        abstraction_definition definition = {};
        definition.call_signature = given.symbols[pointer++].subexpression;
        while (given.symbols[pointer].type != symbol_type::block) {
            definition.return_type.symbols.push_back(given.symbols[pointer++]);
        } definition.body = given.symbols[pointer++].block;

        bool adp_error = adp(definition, stack);
        auto abstraction_type = generate_abstraction_type_for(definition);

        if (debug) {
            std::cout << "about to compare the abstraction types!...\n\n";

            std::cout << "for your infomation, we were given the type ___ to recognize: \n";
            if (expected) {
                print_expression_line(*expected);
            }
            else std::cout << "{{{{TYPE}}}}\n";

            std::cout << "\nand we got the following total type from the abstraction definition, : abstraction_type = \n";
            print_expression_line(*abstraction_type);
            std::cout << "\n\n";
        }

        if ((expressions_match(*expected, *abstraction_type) || expressions_match(*expected, infered_type))) {

            if (debug) {
                std::cout << "we found them to be equal!!\n";
                std::cout << "is_not_type(expected) = " << expected << "\n";
                std::cout << "expressions_match(*expected, *abstraction_type) = " << expressions_match(*expected, *abstraction_type) << "\n";
                std::cout << "expressions_match(*expected, infered_type) = " << expressions_match(*expected, infered_type) << "\n";
            }
            if (expressions_match(*expected, infered_type)) expected = abstraction_type;
            expression result = {{definition}, abstraction_type};
            result.erroneous = adp_error;
            return result;
        } else {
            if (debug) {
                std::cout << "THEY ARE NOT equal!!\n";
                std::cout << "is_not_type(expected) = " << expected << "\n";
                std::cout << "expressions_match(*expected, *abstraction_type) = " << expressions_match(*expected, *abstraction_type) << "\n";
                std::cout << "expressions_match(*expected, infered_type) = " << expressions_match(*expected, infered_type) << "\n";
            }
            pointer = saved;
            delete abstraction_type;
            return {true};
        }
    }
    return {true};
}

expression resolve(std::vector<std::vector<expression>>& stack, expression given, expression& solution_type) {
    auto& list = stack.back();
    std::sort(list.begin(), list.end(), [](auto a, auto b) { return a.symbols.size() > b.symbols.size(); });
    prune_extraneous_subexpressions(given);

    size_t pointer = 0, max_depth = 0;
    expression solution = {};
    auto solution_type_copy = solution_type;
    while (max_depth <= max_expression_depth) {
        pointer = 0;
        solution.type = &solution_type_copy;
        solution = csr(stack, given, 0, max_depth, pointer, solution.type, false);
        if (solution.erroneous or pointer < given.symbols.size() or !solution.type) { max_depth++; }
        else break;
    }
    if (pointer < given.symbols.size() or not solution.type) {
        if (debug) {
            std::cout << "CSR didnt finish parsing the expression or solution type was null, treating as an error...\n";
            std::cout << "solution.type = " << solution.type << "\n";
            std::cout << "pointer < given.symbols.size() = " << (pointer < given.symbols.size()) << "\n";
            std::cout << "solution: "; print_expression_line(solution);
            std::cout << "\n\n";
        }
        solution.erroneous = true;
    }

    if (expressions_match(solution_type, infered_type)) {
        if (solution.type) solution_type = *solution.type; else solution_type = type_type;
    }
    return solution;
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

bool contains_top_level_statements(std::vector<expression> list) {
    for (auto e : list)
        if (not (e.symbols.size() == 1 and e.symbols[0].type == symbol_type::abstraction_definition)) return true;
    return false;
}

translation_unit analyze(translation_unit unit, llvm::LLVMContext& context, struct file file) {

    static bool found_main = false;
    bool error = false;

    wrap_into_main(unit);
    std::vector<std::vector<expression>> stack = {builtins};
    auto& main = unit.list.expressions[0].symbols[0].abstraction;
    auto& body = main.body.list.expressions;

    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size(); i++) {
            auto type = unit_type;
            auto solution = resolve(stack, body[i], type);
            if (solution.erroneous) {
                std::cout << "n3zqx2l: csr: fake error: Could not parse expression!\n"; // TODO: print an error (IN CSR!) of some kind!

                auto type = infered_type;
                auto actual = resolve(stack, body[i], type);

                std::cout << "the actual type was: \n";
                print_expression_line(type);
                std::cout << "\n\n";

                std::cout << "was expecting: \n";
                print_expression_line(unit_type);
                std::cout << "\n\n";

                error = true;
                continue;
            }
            parsed_body.push_back(solution);
        }
        main.body.list.expressions = parsed_body;
        if (contains_top_level_statements(parsed_body)) {
            if (found_main) {
                std::cout << "n3zqx2l: fake error: cannot have multiple files with top level statements.\n"; // TODO: print an error of some kind!
            } else {
                file.is_main = true;
                found_main = true;
            }
        }
    } else main.return_type = unit_type;

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
