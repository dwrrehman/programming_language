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


/*   --------- KNOWN BUGS: -------------

    _type


 ---> csr error.

---------------------

    print abs () {
        dog
    }

 ---> succeeds, when it shouldnt,

---------------------












TODO:




        DEBUG:

                make a quick and simple expression parser, which
                prints it on one line, very simply, using parens for subexpressions.





        FUNC:

                - basic ADP

                - literal recognition

                - ast nodes

                - FDI



                - make it so that the user can write a return type of _none, and then have the compiler NOT make the last statement a return statemnet. (obviously because it will fail for every def of that func)




        ERROR MESSAGES:

                - better EM for a extraneous ")", in parser.

                - better EM for unresolved expression,

                - make an error queue, which error messages are depositied onto, (with a limit of the number of errors)
                    and then pritn the whole stack at the end of the programs compilation, with the error count.







 */




/// Global builtin types. these are fundemental to the language:
expression type_type = {{{"_type", false}}};
expression none_type = {{{"_none", false}}, &type_type};
expression unit_type = {{}, &type_type};
expression infered_type = {{{"_infered", false}}};

expression i32_type = {{{"_i32", false}}, &type_type};
expression exit_abstraction = {                 // used for returning from main, always.
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

expression abs_type = {
    {
        {"_abstraction", false},
        {{{}, &unit_type}},
    }, &type_type};

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
//
//expression unit_to_int_type = {
//    {
//        {{{}, &unit_type}}
//    }, &int_type};
//
//expression int_to_unit_type = {
//    {
//        {{{}, &int_type}}
//    }, &unit_type};

////////////////////////////////////////////////////////////////////




std::vector<expression> builtins =  {
    unit_type, none_type, infered_type, i32_type, exit_abstraction,

    // TESTING:
    int_type, dog_type, print_type, //int_to_unit_type, unit_to_int_type
    is_good_type, x_type, int0_literal,

    abs_type, print_abs_type,
};



bool expressions_match(expression first, expression second, const int d);
expression csr(std::vector<std::vector<expression>>& stack, const expression given, const size_t depth, const size_t max_depth, size_t& pointer, struct expression*& type);
bool adp(abstraction_definition& given, std::vector<std::vector<expression>>& stack);
expression resolve(std::vector<std::vector<expression>>& stack, expression given, expression& solution_type);

expression* is_not_type(expression* type) {
    if (type) {
//        std::cout << "LOWERING:::\n";
        if (expressions_match(*type, type_type, 0)) {
            return nullptr;
        } else return type;
    }
    return nullptr;
}

bool symbols_match(symbol first, symbol second, const int d) {
//    prep(d); std::cout << "comparing symbols....\n";
    if (first.type == symbol_type::subexpression && second.type == symbol_type::subexpression
        && expressions_match(first.subexpression, second.subexpression, d + 1)) {
//        prep(d); std::cout << "they are subexprs! comparing subexprs....\n";
        return true;
    } else if (first.type == symbol_type::identifier && second.type == symbol_type::identifier
             && first.identifier.name.value == second.identifier.name.value) {
//        std::cout << "idnetifier symbols are equal: "<< second.identifier.name.value <<"\n";
        return true;
    } else {
//        prep(d); std::cout << "symbols are not equal.\n";
        return false;
    }
}

bool expressions_match(expression first, expression second, const int d) {

    prep(d); std::cout << "---- CALLED EXPRESSIONS MATCH ---------------\n";
    prep(d);std::cout << "first: \n";
    print_expression(first, d+1);
    std::cout << "\n\n";
    prep(d);std::cout << "second\n";
    print_expression(second, d+1);
    std::cout << "\n\n";

//    prep(d); std::cout << "comparing first and second sizes...\n";
    if (first.symbols.size() != second.symbols.size()) return false;
//    prep(d); std::cout << "they were equal!\n";
    for (size_t i = 0; i < first.symbols.size(); i++) {
        if (!symbols_match(first.symbols[i], second.symbols[i], d+1)) return false;
//        std::cout << "their symbol were equal!\n";
    }
    if (first.erroneous || second.erroneous) return false;
//    prep(d); std::cout << "both are not erroreous :)\n";
//
    prep(d); std::cout << "comparing their types: \n";
    if (first.type) print_expression(*first.type, d+1);
    else {prep(d); std::cout << "{{{TYPE}}}\n";}
    std::cout << "---------- and -----------\n";
    if (second.type) print_expression(*second.type, d+1);
    else {prep(d); std::cout << "{{{TYPE}}}\n";}
    std::cout << "\n";

//    if ((!is_not_type(first.type) && !is_not_type(second.type))
//        || (is_not_type(first.type) && is_not_type(second.type) && expressions_match(*first.type, *second.type, d+1))) {
//        prep(d);std::cout << "the types matched!\n";
    //if (is_not_type(first.type) == is_not_type(second.type)) {

    if (!is_not_type(first.type) && !is_not_type(second.type)) {
        prep(d);std::cout << "the types matched!\n";
        return true;

    } else if (is_not_type(first.type) && is_not_type(second.type) && expressions_match(*first.type, *second.type, d+1)) {
        prep(d);std::cout << "the types matched!\n";
        return true;

    } else {
        prep(d); std::cout << "the types didnt match...\n";
        return false;
    }
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
    type->is_new = true;

    type->symbols.push_back({"_abstraction", false}); // temp, debug
    auto parameter_list = filter_subexpressions(def.call_signature);
    for (auto parameter : parameter_list) {
        expression t = type_type;
        if (parameter.subexpression.type) t = *parameter.subexpression.type;
        type->symbols.push_back(t);
    }
    type->symbols.push_back({def.return_type});
    return type;
}


void clean(block& body) {

    std::cout << "block body before cleaning:\n";
    print_block(body, 0);

    block result = {};
    for (auto expression : body.list.expressions) {
        if (!expression.symbols.empty()) {
            result.list.expressions.push_back(expression);
        }
    }
    body = result;


    std::cout << "block body AFTER cleaning:\n";
    print_block(body, 0);

}


bool adp(abstraction_definition& given, std::vector<std::vector<expression>>& stack) {
    if (debug) {
        std::cout << "----------- printing what abs adp got: ----------- \n";
        print_abstraction_definition(given, 0);
    }

    stack.push_back(stack.back());


//notes for my fufure self:

        // when we parse the parameters and the rt, we need to pass "true" into resolve(). this is for whetheer we should allow for fdi.

        /// although it isnt exactly fdi in this case, its actually just letting us know that we are allowing for a signature which hasnt been defined yet.

        // this is for cases such as when we want to allow for type generic parameters. we want to allow the user to define a new type by definiing a new variable from nothing, like haskell.

        // same for the rt.

            // important! we need to actually define these variables in this function OURSELSES, (add them into list, via calling our own define_signature(...)) and we do this after every parameter ie, things defined by one parameter can be used by the next parameter. this is cricual.

            // finally, we need to make sure that



    /// STEP 1: parse the signature.




    /// STEP 2: parse the return type.

    if (given.return_type.symbols.size()) {
        given.return_type = resolve(stack, given.return_type, infered_type);
    } else {
        given.return_type = infered_type;
    }



    // STEP 3: parse the body.

    clean(given.body);
    auto& body = given.body.list.expressions;

    bool error = false;

    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size() - 1; i++) {
            auto solution = resolve(stack, body[i], unit_type);
            if (solution.erroneous) {
                std::cout << "n3zqx2l: adp-csr: fake error: Could not parse expression!\n"; // TODO: print an error (IN CSR!) of some kind!
                error = true;
                continue;
            }
            parsed_body.push_back(solution);
        }
        auto solution = resolve(stack, body[body.size() - 1], given.return_type);
        if (solution.erroneous) {
            std::cout << "n3zqx2l: adp-csr: fake error: Could not parse return expression!\n"; // TODO: print an error (IN CSR!) of some kind!
            error = true;
        }
        parsed_body.push_back(solution);
        given.body.list.expressions = parsed_body;
    } else {
        given.return_type = type_type;
    }

    stack.pop_back();
    return error;
}

bool contains_a_block_starting_from(size_t begin, std::vector<symbol> list) {
    for (; begin < list.size(); begin++) {
        if (list[begin].type == symbol_type::block) {
            return true;
        }
    }
    return false;
}



expression csr(std::vector<std::vector<expression>>& stack, const expression given, const size_t depth, const size_t max_depth, size_t& pointer, struct expression*& expected) {
    const auto list = stack.back();
    if (depth > max_depth) return {true};
    if (is_not_type(expected) && expressions_match(*expected, none_type, 0)) return {true};
    if (given.symbols.empty() || (given.symbols.size() == 1
                                   && given.symbols[0].type == symbol_type::subexpression
                                   && given.symbols[0].subexpression.symbols.empty())) {
        if (given.symbols.size() == 1
            && given.symbols[0].type == symbol_type::subexpression
            && given.symbols[0].subexpression.symbols.empty()) pointer++;
        if (is_not_type(expected) && expressions_match(*expected, infered_type, 0)) expected = &unit_type;
        if (is_not_type(expected) && expressions_match(*expected, unit_type, 0)) return {{}, &unit_type};
        else return {true};
    }
    const size_t saved = pointer;
    for (auto signature : list) {
        if (is_not_type(expected) && !expressions_match(*expected, infered_type, 0) && (!signature.type || !expressions_match(*expected, *signature.type, 0))) continue;
        expression solution = {};
        pointer = saved;
        bool failed = false;
        for (auto element : signature.symbols) {
            if (pointer >= given.symbols.size()) { failed = true; break; }
            if (element.type == symbol_type::subexpression) {
                auto subexpression = csr(stack, given, depth + 1, max_depth, pointer, element.subexpression.type);
                if (subexpression.erroneous) {
                    if (given.symbols[pointer].type == symbol_type::subexpression) {
                        size_t local_pointer = 0, current_depth = 0;
                        expression subexpression = {};
                        while (current_depth <= max_expression_depth) {
                            local_pointer = 0;
                            subexpression = csr(stack, given.symbols[pointer].subexpression, 0, current_depth, local_pointer, element.subexpression.type);
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
            if (is_not_type(expected) && expressions_match(*expected, infered_type, 0)) expected = signature.type;
            solution.type = signature.type;
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
            std::cout << "found an abstraction, it has type: \n";
            print_expression(*abstraction_type, 0);

            std::cout << "for your infomation, we were given the type ___ to recognize: \n";
            if (expected) print_expression(*expected, 0);
            else std::cout << "{{{{TYPE}}}}\n";

            std::cout << "\nand we got the following total type from the abstraction definition, :\n";
            print_expression(*abstraction_type, 0);
            std::cout << "\n\n";
        }

        if (is_not_type(expected) && (expressions_match(*expected, *abstraction_type, 0) || expressions_match(*expected, infered_type, 0))) {

            std::cout << "we found them to be equal!!\n";
            std::cout << "is_not_type(expected) = " << is_not_type(expected) << "\n";
            std::cout << "expressions_match(*expected, *abstraction_type) = " << expressions_match(*expected, *abstraction_type, 0) << "\n";
            std::cout << "expressions_match(*expected, infered_type) = " << expressions_match(*expected, infered_type, 0) << "\n";

            if (is_not_type(expected) && expressions_match(*expected, infered_type, 0)) expected = abstraction_type;
            expression result = {{definition}, abstraction_type};
            result.erroneous = adp_error;
            return result;
        } else { pointer = saved; return {true}; }
    }
    return {true};
}

expression resolve(std::vector<std::vector<expression>>& stack, expression given, expression& solution_type) {
    auto& list = stack.back();
    std::sort(list.begin(), list.end(), [](auto a, auto b) { return a.symbols.size() > b.symbols.size(); });
    prune_extraneous_subexpressions(given);

    if (debug) {
        std::cout << "printing given list:\n";
        for (auto l : list) {
            print_expression(l, 0);
            std::cout << "\n- -\n";
        }
        std::cout << "finished printing list..\n";

        std::cout << "given expression:\n";
        print_expression(given, 0);
        std::cout << "\n\n\n";
    }

    size_t pointer = 0, max_depth = 0;
    expression solution = {};
    auto solution_type_copy = solution_type;
    while (max_depth <= max_expression_depth) {
        pointer = 0;
        solution.type = &solution_type_copy;
        solution = csr(stack, given, 0, max_depth, pointer, solution.type);
        if (solution.erroneous || pointer < given.symbols.size()) { max_depth++; }
        else break;
    }
    if (pointer < given.symbols.size()) solution.erroneous = true;

    if (expressions_match(solution_type, infered_type, 0) && solution.type) {
        solution_type = *solution.type;
    } else if (expressions_match(solution_type, infered_type, 0) && !solution.type) {
        solution_type = type_type;
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
    for (auto e : list) {
        if (!(e.symbols.size() == 1 && e.symbols[0].type == symbol_type::abstraction_definition)) {
            return true;
        }
    }
    return false;
}

translation_unit analyze(translation_unit unit, struct file file) {

    expression first = {{
        {"_abstraction", false},
        {{{}, &int_type}}
    }, nullptr};
    expression second = abs_type;


        std::cout << "first: \n";
        print_expression(first, 0);
        std::cout << "\n\n";
        std::cout << "second\n";
        print_expression(second, 0);
        std::cout << "\n\n";


    if (expressions_match(first, second, 0)) {
        std::cout << "they match!\n";
    } else {
        std::cout << "they DONT match!\n";
    }


    return {};















    static bool found_main = false;

    wrap_into_main(unit);
    std::vector<std::vector<expression>> stack = {builtins};
    auto& main = unit.list.expressions[0].symbols[0].abstraction;
    auto& body = main.body.list.expressions;

    bool error = false;

    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size(); i++) {
            auto solution = resolve(stack, body[i], unit_type);
            if (solution.erroneous) {
                std::cout << "n3zqx2l: csr: fake error: Could not parse expression!\n"; // TODO: print an error (IN CSR!) of some kind!
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
