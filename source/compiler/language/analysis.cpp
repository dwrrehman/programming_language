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



#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ADT/Optional.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/MemoryBuffer.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include <sstream>
#include <iostream>

/*----------------------- KNOWN BUGS ------------------------

 (int) {}
 (g) int {}
 (hello (x)) {
    hello x
    hello g
 }

 ---> succeeds, but doesnt fill in the infered type for x, in the call signature for hello, only in the call to hello using g.





------------------------- TODOs ----------------------------



                want something to do?



    0. implement LLVM types.

    0.1. allow llvm interaction with variables, back and forth.

    1. implement FDI for CSR.

    2. implement _scope1, _scope0, etc.

    3. implement "_define" and "_undefine" / "_undefine all"

    4. implement _level0, _level1, etc.

    5. implement NSS for ADP.parse_signature

    6. implement userdefined precedence and associavity




    N. implement the better error printing/handling system.










 */


/// Global builtin types. these are fundemental to the language:
expression type_type = {{{"_type", false}}};
expression unit_type = {{}, &type_type};
expression none_type = {{{"_none", false}}, &type_type};
expression infered_type = {{{"_infered", false}}};

std::vector<expression> builtins =  {
    type_type, unit_type, none_type, infered_type,
};


bool expressions_match(expression first, expression second);
expression csr(std::vector<std::vector<expression>>& stack, const expression given, const size_t depth, const size_t max_depth, size_t& pointer, struct expression*& expected, bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type);
bool adp(abstraction_definition& given, std::vector<std::vector<expression>>& stack);
expression resolve(std::vector<std::vector<expression>>& stack, expression given, expression& expected_solution_type, bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type);

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
    } return result;
}

expression* generate_abstraction_type_for(abstraction_definition def) {
    auto type = new expression();        ///TODO: free this at some point.
    type->was_allocated = true;
    auto parameter_list = filter_subexpressions(def.call_signature);
    if (parameter_list.empty()) {
        *type = def.return_type;
        return type;
    }
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
    stack.back().push_back(given.call_signature);
    auto& body = given.body.list.expressions;
    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size() - 1; i++) {
            auto type = unit_type;
            auto solution = resolve(stack, body[i], type, false, true, false);
            if (solution.erroneous) {
                std::cout << "n3zqx2l: adp-csr: fake error: Could not parse expression!\n"; // TODO: print an error (IN CSR!) of some kind!

                auto type = infered_type;
                auto actual = resolve(stack, body[i], type, false, true, false);

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
        auto solution = resolve(stack, body[body.size() - 1], given.return_type, false, true, false);
        if (solution.erroneous) {
            std::cout << "n3zqx2l: adp-csr: fake error: Could not parse return expression!\n"; // TODO: print an error (IN CSR!) of some kind!

            auto type = infered_type;
            auto actual = resolve(stack, body[body.size() - 1], type, false, true, false);

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
    if (given.call_signature.type) *given.call_signature.type = given.return_type;
}

static void parse_return_type(abstraction_definition &given, std::vector<std::vector<expression> > &stack, bool& error) {
    ///TODO: this function needs to evaluate its return type, at compiletime.
    if (given.return_type.symbols.size()) {
        auto type = infered_type;
        given.return_type = resolve(stack, given.return_type, type, true, false, true);
        if (given.return_type.erroneous) {
            std::cout << "n3zqx2l: adp-csr: fake error: Could not parse return type.\n"; // TODO: print an error (IN CSR!) of some kind!
            error = true;
        }
        if (given.return_type.type and expressions_match(*given.return_type.type, unit_type) and given.return_type.symbols.empty()) given.return_type = unit_type;
        else if (given.return_type.type and expressions_match(*given.return_type.type, none_type)) given.return_type = none_type;
    } else given.return_type = infered_type;
    given.call_signature.type = new expression();
    *given.call_signature.type = given.return_type;
    given.call_signature.type->was_allocated = true;
}

static void parse_signature(abstraction_definition &given, std::vector<std::vector<expression>>& stack, bool& error) {
    expression result = {};
    auto call = given.call_signature.symbols;
    for (size_t i = 0; i < call.size(); i++) {
        if (call[i].type == symbol_type::subexpression) {
            auto sub = call[i].subexpression;
            abstraction_definition definition = {};
            size_t pointer = 0;
            if (sub.symbols.empty()) continue;
            else if (sub.symbols[pointer].type == symbol_type::subexpression) {
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
                auto parameter_type = new expression();
                *parameter_type = infered_type;
                parameter_type->was_allocated = true;
                expression parameter = {sub.symbols, parameter_type};
                result.symbols.push_back({parameter});
                stack.back().push_back(parameter);
            }
        } else if (call[i].type == symbol_type::identifier) {
            result.symbols.push_back(call[i]);

        } else { //TODO: add additional cases for the other symbol types. (like strings, etc.)
            error = true;
        }
    }
    given.call_signature = result;
}

bool adp(abstraction_definition& given, std::vector<std::vector<expression>>& stack) {
    clean(given.body); //TODO: move me into the corrector code.
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







llvm::Type* parse_llvm_string_as_type(std::string given) {
    std::cout << "received LLVM type: ";
    std::cout << given;
    std::cout << "\n";
    return nullptr;
}

llvm::Instruction* parse_llvm_string_as_instruction(std::string given) {
    std::cout << "received LLVM instruction: ";
    std::cout << given;
    std::cout << "\n";
    return nullptr;
}

llvm::Function* parse_llvm_string_as_function(std::string given) {
    std::cout << "received LLVM function: ";
    std::cout << given;
    std::cout << "\n";
    return nullptr;
}


/*
 might be useful:


        llvm_function->copyAttributesFrom(const Function *Src)

 */


































std::string random_string() {
    static int num = 0;
    std::stringstream stream;
    stream << std::hex << rand();
    return std::string(stream.str()) + std::to_string(num++);
}


static void otherLoop() {
    std::string code = "";

    while (true) {
        std::cout << "::> ";

        std::string mode = "";
        std::cin >> mode;

        llvm::SMDiagnostic errors;

        if (mode == "expr") {

            std::getline(std::cin, code);
            std::cout << "received: \"" << code << "\"\n";

            if (code == "done" || code == "") {
                std::cout << "quitting...\n";
                break;
            }

            if (code.size() && code[0]) {
                code.insert(0, "define void @_anonymous_" + random_string() + "() {\n"); // wrap the given llvm statements in a function, named the same as the function we want to insert these statements into.
                code += "\nret void\n}\n";
            }

            for (int i = 0; i < code.size(); i++) {    // temp, to make the cli more useable.
                if (code[i] == '/') {
                    code[i] = '\n';
                }
            }

            llvm::MemoryBufferRef reference(code, "temp_ins_buffer");
            llvm::ModuleSummaryIndex my_index(true);
            llvm::SMDiagnostic errors;

            if (llvm::parseAssemblyInto(reference, TheModule.get(), &my_index, errors)) {
                std::cout << "\nparsed unsuccessfully.\n\n";


                std::cout << "llvm: "; // TODO: make this have color!
                errors.print("MyProgram.n", llvm::errs());

            } else {
                std::cout << "\nparsed successfully.\n\n";
            }
        } else if (mode == "decl") {

            std::getline(std::cin, code);
            std::cout << "received: \"" << code << "\"\n";

            if (code == "done" || code == "") {
                std::cout << "quitting...\n";
                break;
            }

            for (int i = 0; i < code.size(); i++) {    // temp, to make the cli more useable.
                if (code[i] == '/') {
                    code[i] = '\n';
                }
            }

            llvm::MemoryBufferRef reference(code, "temp_ins_buffer");
            llvm::ModuleSummaryIndex my_index(true);
            llvm::SMDiagnostic errors;

            if (llvm::parseAssemblyInto(reference, TheModule.get(), &my_index, errors)) {
                std::cout << "\nparsed unsuccessfully.\n\n";

                std::cout << "llvm: "; // TODO: make this have color!
                errors.print("MyProgram.n", llvm::errs());

            } else {
                std::cout << "\nparsed successfully.\n\n";
            }

        } else if (mode == "show") {
            std::cout << "printing all functions:\n";
            for (auto& function : TheModule->functions()) {
                std::cout << "printing function:\n";
                function.llvm::Value::print(llvm::errs());
                std::cout << "done printing function!\n";
            }
            std::cout << "done printing!\n";

        } else if (mode == "type") {

            std::getline(std::cin, code);

            code.erase(code.begin());
            std::cout << "received: \"" << code << "\"\n";

            if (code == "done" || code == "") {
                std::cout << "quitting...\n";
                break;
            }

            for (int i = 0; i < code.size(); i++) {    // temp, to make the cli more useable.
                if (code[i] == '/') {
                    code[i] = '\n';
                }
            }

            llvm::MemoryBufferRef reference(code, "temp_ins_buffer");
            llvm::ModuleSummaryIndex my_index(true);
            llvm::SMDiagnostic errors;

            llvm::Type* type;

            if ((type = llvm::parseType(code, errors, *TheModule)) != nullptr) {
                std::cout << "succcesfully parsed type: \n";
                type->print(llvm::outs());
            } else {
                std::cout << "\n\nllvm: "; // TODO: make this have color!
                errors.print("MyProgram.n", llvm::errs());
                std::cout << "\n\n";
            }

        } else if (mode == "print") {
            std::cout << "printing the results: \n\n";
            TheModule->print(llvm::outs(), nullptr);

        } else if (mode == "help") {
            std::cout << "say: print or type or decl or expr.\n";
        }
        code = "";
    }

    std::cout << "printing the results: \n\n";
    TheModule->print(llvm::outs(), nullptr);
}



































expression csr(std::vector<std::vector<expression>>& stack, const expression given, const size_t depth, const size_t max_depth, size_t& pointer, struct expression*& expected, bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type) {
    if (depth > max_depth) return {true};
    if (!expected) return {true};
    if (given.symbols.empty() or (given.symbols.size() == 1
                                  and given.symbols[0].type == symbol_type::subexpression
                                  and given.symbols[0].subexpression.symbols.empty())) {
        if (given.symbols.size() == 1
            and given.symbols[0].type == symbol_type::subexpression
            and given.symbols[0].subexpression.symbols.empty()) pointer++;
        if (expressions_match(*expected, infered_type)) expected = &unit_type;
        if (expressions_match(*expected, unit_type)) return unit_type;
        else return {true};
    } else if (given.symbols.size() == 1 and given.symbols[0].type == symbol_type::llvm_literal) {

        auto llvm_string = given.symbols[0].llvm.literal.value;

        if (is_at_top_level and not is_parsing_type) {
            auto llvm_string = given.symbols[0].llvm.literal.value;

            if (auto llvm_instruction = parse_llvm_string_as_instruction(llvm_string)) {

                expression solution = {};
                solution.erroneous = false;
                solution.llvm_instruction = llvm_instruction;

                


                solution.symbols = {};
                solution.type = &unit_type;

                return solution;

            } else if (auto llvm_function = parse_llvm_string_as_function(llvm_string)) {

                expression solution = {};
                solution.erroneous = false;
                solution.llvm_function = llvm_function;


                solution.type = &unit_type;
                solution.symbols = {};

                return solution;

            } else {
                // print error, assuming instruction, as well as for function.
                return {true};
            }

        } else if (is_parsing_type and not is_at_top_level) {
            if (auto llvm_type = parse_llvm_string_as_type(llvm_string)) {

                expression solution = {};
                solution.erroneous = false;
                solution.llvm_type = llvm_type;
                solution.symbols = {};

                return solution;

            }
        } else {
            std::cout << "unexpected llvm string here.\n"; // TODO: make this a proper error.
        }
    }

    const auto saved = pointer;
    for (auto& signature : stack.back()) {
        if (not expressions_match(*expected, infered_type) and signature.type and not expressions_match(*signature.type, infered_type) and not expressions_match(*expected, *signature.type)) continue;
        expression solution = {};
        pointer = saved;
        auto failed = false;
        for (auto& element : signature.symbols) {
            if (pointer >= given.symbols.size()) { failed = true; break; }
            if (element.type == symbol_type::subexpression) {
                auto& TEMP = element.subexpression.type;
                auto subexpression = csr(stack, given, depth + 1, max_depth, pointer, TEMP, /*bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type*/ can_define_new_signature, false, is_parsing_type);
                if (subexpression.erroneous) {
                    pointer = saved;
                    if (given.symbols[pointer].type == symbol_type::subexpression) {
                        size_t local_pointer = 0, current_depth = 0;
                        expression subexpression = {};
                        while (current_depth <= max_expression_depth) {
                            local_pointer = 0;
                            auto& TEMP = element.subexpression.type;
                            subexpression = csr(stack, given.symbols[pointer].subexpression, 0, current_depth, local_pointer, TEMP, /*bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type*/ can_define_new_signature, false, is_parsing_type);
                            if (subexpression.erroneous or local_pointer < given.symbols[pointer].subexpression.symbols.size()) {
                                current_depth++;
                            } else break;
                        }
                        if (subexpression.erroneous or local_pointer < given.symbols[pointer].subexpression.symbols.size()) { failed = true; break; }
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
            if (not expressions_match(*expected, infered_type) and signature.type
                and expressions_match(*signature.type, infered_type)) *signature.type = *expected;
            else if (expressions_match(*expected, infered_type)) expected = signature.type;
            if (signature.type) solution.type = signature.type;
            else solution.type = &type_type;
            return solution;
        }
    }
    if (given.symbols[pointer].type == symbol_type::subexpression and contains_a_block_starting_from(pointer + 1, given.symbols)) {
        abstraction_definition definition = {};
        definition.call_signature = given.symbols[pointer++].subexpression;
        while (given.symbols[pointer].type != symbol_type::block) {
            definition.return_type.symbols.push_back(given.symbols[pointer++]);
        } definition.body = given.symbols[pointer++].block;
        bool adp_error = adp(definition, stack);

        auto abstraction_type = generate_abstraction_type_for(definition);
        prune_extraneous_subexpressions(*abstraction_type);

        if ((expressions_match(*expected, *abstraction_type) or expressions_match(*expected, infered_type)) or is_at_top_level) {
            if (expressions_match(*expected, infered_type)) expected = abstraction_type;
            expression result = {{definition}, abstraction_type};
            result.erroneous = adp_error;
            if (is_at_top_level) {
                *result.type = unit_type;
                stack.back().push_back(definition.call_signature); ///TODO: make this go through a central function, "add_signature_to_symbol_table(sig, stack)".
            }
            return result;
        } else {
            pointer = saved;
            delete abstraction_type;
            return {true};
        }
    }
    return {true};
}

expression resolve(std::vector<std::vector<expression>>& stack, expression given, expression& expected_solution_type, bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type) {
    auto& list = stack.back();
    std::sort(list.begin(), list.end(), [](auto a, auto b) { return a.symbols.size() > b.symbols.size(); });
    prune_extraneous_subexpressions(given);
    size_t pointer = 0, max_depth = 0;
    expression solution = {};
    while (max_depth <= max_expression_depth) {
        pointer = 0;
        auto solution_type_copy = expected_solution_type;
        solution.type = &solution_type_copy;
        solution = csr(stack, given, 0, max_depth, pointer, solution.type, /*bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type*/ can_define_new_signature, is_at_top_level, is_parsing_type);
        if (solution.erroneous or pointer < given.symbols.size() or not solution.type) { max_depth++; }
        else break;
    }
    if (pointer < given.symbols.size() or not solution.type) solution.erroneous = true;

    if (not solution.erroneous and expressions_match(solution, unit_type)) {
        solution.type = &unit_type;
    }
    if (expressions_match(expected_solution_type, infered_type)) {
        if (solution.type) {
            expected_solution_type = *solution.type;
        }
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

    auto type = type_type;

    auto& main = unit.list.expressions[0].symbols[0].abstraction;
    auto& body = main.body.list.expressions;

    std::cout << "starting with stack frame: \n";
    print_stack(stack);

    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size(); i++) {
            auto type = unit_type;
            auto solution = resolve(stack, body[i], type, false, true, false);
            if (solution.erroneous) {
                std::cout << "n3zqx2l: csr: fake error: Could not parse expression!\n"; // TODO: print an error (IN CSR!) of some kind!

                auto type = infered_type;
                auto actual = resolve(stack, body[i], type, false, true, false);

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

    std::cout << "stack is now: \n";
    print_stack(stack);

    if (error) {
        std::cout << "\n\n\tCSR ERROR\n\n\n\n";
        throw "analysis error";
    } else {
        std::cout << "\n\n\tsuccess.\n\n\n";
        return unit;
    }
}
