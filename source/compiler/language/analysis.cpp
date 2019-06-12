//
//  analysis.cpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//
/*
 -------------------------------whats missing?------------------------------------------
 
 

 -----> code gen for abstractions
 
 -----> compiletime evaluation
 
 
 -----> scope/func references:
     
                _abstraction_0_1
 
                _application_2_2
 
 
 
 
 -----> write a custom stack class, 
 
                which keep tracks if something was defined in the current scope or a parent scope. 

        
----->  complete the FDI algorithm in CSR
 
 
 -----> how to implement the "_recognize <_signature>"
 
 
 
 
 
 */




#include "analysis.hpp"
#include "parser.hpp"
#include "nodes.hpp"
#include "lists.hpp"
#include "builtins.hpp"
#include "debug.hpp"
#include "helpers.h"

#include "llvm/IR/ValueSymbolTable.h"
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
#include "llvm/IR/ValueMap.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
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
#include "llvm/Transforms/Utils/FunctionComparator.h"

#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/AsmParser/Parser.h"

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


bool add_signature_to_symbol_table(expression new_signature, stack& stack) {    
    stack.back().push_back(new_signature); //TODO: should we do anything else?
    return false;
}

static void parse_abstraction_body(abstraction_definition &given, stack &stack, tu_data& data, flags flags, bool& error) {        
    add_signature_to_symbol_table(given.call_signature, stack);
    auto& body = given.body.list.expressions;    
    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size() - 1; i++) {
            
            auto type = unit_type;
            auto solution = resolve(body[i], type, stack, data, flags
                                .dont_allow_undefined()
                                .at_top_level()
                                .not_parsing_a_type(), error);
            
            if (solution.erroneous) error = true;                
            else parsed_body.push_back(solution);
        }
        
        auto solution = resolve(body[body.size() - 1], given.return_type, stack, data, flags
                            .dont_allow_undefined()
                            .at_top_level()
                            .not_parsing_a_type(), error);        
        if (solution.erroneous) error = true;
        else parsed_body.push_back(solution);
        given.body.list.expressions = parsed_body;
        
    } else if (expressions_match(given.return_type, infered_type)) given.return_type = unit_type;
    else if (not expressions_match(given.return_type, unit_type)) error = true;    
    if (given.call_signature.type) *given.call_signature.type = given.return_type;
}

static void parse_return_type(abstraction_definition &given, stack &stack, tu_data& data, flags flags, bool& error) {
    
    if (given.return_type.symbols.size()) {
        auto type = infered_type;
        
        given.return_type = resolve(given.return_type, type, stack, data, flags
                                .allow_undefined()
                                .not_at_top_level()
                                .parsing_a_type(), error);
        if (given.return_type.erroneous) error = true;
        
        if (given.return_type.type and expressions_match(*given.return_type.type, unit_type) and given.return_type.symbols.empty()) given.return_type = unit_type;
        else if (given.return_type.type and expressions_match(*given.return_type.type, none_type)) given.return_type = none_type;
    } else given.return_type = infered_type;
    
    given.call_signature.type = new expression();
    *given.call_signature.type = given.return_type;
    given.call_signature.type->was_allocated = true;
}

static void push_infered_parameter(expression &result, stack &stack, const expression &sub) {
    auto parameter_type = new expression();
    *parameter_type = infered_type;
    parameter_type->was_allocated = true;
    expression parameter = {sub.symbols, parameter_type};
    result.symbols.push_back({parameter});
    stack.back().push_back(parameter);
}

void parse_signature(abstraction_definition &given, stack &stack, tu_data& data, flags flags, bool& error);

static void push_typed_parameter(tu_data &data, bool &error, const flags &flags, expression &result, stack &stack, expression &sub) {
    abstraction_definition definition = {};
    size_t pointer = 0;
    definition.call_signature = sub.symbols[pointer++].subexpression;
    while (pointer < sub.symbols.size()) definition.return_type.symbols.push_back(sub.symbols[pointer++]);                
    parse_signature(definition, stack, data, flags, error); 
    parse_return_type(definition, stack, data, flags, error);
    auto parameter_type = generate_abstraction_type_for(definition);
    expression parameter = {definition.call_signature.symbols, parameter_type};
    result.symbols.push_back({parameter});
    stack.back().push_back(parameter);
}

void parse_signature(abstraction_definition &given, stack &stack, tu_data& data, flags flags, bool& error) {
    expression result = {};
    auto call = given.call_signature.symbols;
    for (size_t i = 0; i < call.size(); i++) {
        if (subexpression(call[i])) {
            auto sub = call[i].subexpression;
            if (sub.symbols.empty()) continue;
            else if (subexpression(sub.symbols.front()))
                push_typed_parameter(data, error, flags, result, stack, sub);
            else push_infered_parameter(result, stack, sub);
            
        } else if (identifier(call[i])) {
            result.symbols.push_back(call[i]);
        } else error = true;        
    }
    given.call_signature = result;
}

void parse_abstraction(abstraction_definition& given, stack& stack, tu_data& data, flags flags, bool& error) {
    clean(given.body);
    stack.push_back(stack.back());
    parse_signature(given, stack, data, flags, error);
    parse_return_type(given, stack, data, flags, error);
    parse_abstraction_body(given, stack, data, flags, error);
    stack.pop_back();
}

static void use_csr1(tu_data &data, symbol &element, bool &error, expression &fdi, flags &flags, const expression &given, size_t &local_pointer, size_t &pointer, stack &stack, expression &subexpression) {
    subexpression = expression {};
    local_pointer = 0; size_t current_depth = 0;    
    while (current_depth <= max_expression_depth) {
        local_pointer = 0;
        auto& type = element.subexpression.type;
        flags.should_allow_undefined_signatures = type and expressions_match(*type, signature_type);
        subexpression = csr(given.symbols[pointer].subexpression,  type, fdi, 0, current_depth, local_pointer, stack, data, flags.not_at_top_level(), error);
        if (subexpression.erroneous or local_pointer < given.symbols[pointer].subexpression.symbols.size()) {
            current_depth++;
        } 
        else break;
    }
}
// call it "descend()"?
static void use_csr2(tu_data &data, bool &error, expression &expected_type, expression &fdi, flags &flags, const expression &given, size_t &pointer, expression &solution, stack &stack) {
    solution = expression {};
    pointer = 0; size_t max_depth = 0;
    while (max_depth <= max_expression_depth) {
        pointer = 0;
        auto copy = expected_type;
        solution.type = &copy;
        flags.should_allow_undefined_signatures = expressions_match(*solution.type, signature_type);
        solution = csr(given, solution.type, fdi, 0, max_depth, pointer, stack, data, flags, error);
        if (solution.erroneous or pointer < given.symbols.size() or not solution.type) { 
            max_depth++;
        }
        else break;
    }
}

 expression parse_new_abstraction_definition(tu_data &data, bool &error, expression *&expected, const flags &flags, const expression &given, size_t &pointer, unsigned long saved, stack &stack) {
    abstraction_definition definition = preliminary_parse_abstraction(given, pointer);        
    parse_abstraction(definition, stack, data, flags, error);
    auto abstraction_type = generate_abstraction_type_for(definition);
    prune_extraneous_subexpressions(*abstraction_type);
    
    if ((expressions_match(*expected, *abstraction_type) or expressions_match(*expected, infered_type)) or flags.is_at_top_level) {
        if (expressions_match(*expected, infered_type)) expected = abstraction_type;
        expression result = {{definition}, abstraction_type};
        result.erroneous = error;
        if (not result.erroneous and flags.is_at_top_level) {
            *result.type = unit_type;
            add_signature_to_symbol_table(definition.call_signature, stack);
        }
        return result;
    } else {
        pointer = saved;
        delete abstraction_type;
        return {true};
    }
}

expression csr(const expression& given, expression*& expected, expression& fdi, const size_t depth, const size_t max_depth, size_t& pointer, stack& stack, tu_data& data, flags flags, bool& error ) {    
    if (depth > max_depth or not expected) return {true};    
    if (found_unit_expression(given)) return parse_unit_expression(expected, given, pointer);
    else if (found_llvm_string(given, pointer)) return parse_llvm_string(given, given.symbols[pointer].llvm.literal.value, pointer, stack, data, flags, error);
    
    const auto saved = pointer;
    for (auto& signature : stack.back()) {
        
        if (not expressions_match(*expected, infered_type) and signature.type
            and not expressions_match(*signature.type, infered_type) 
            and not expressions_match(*expected, *signature.type)) continue;
        
        expression solution = {};
        pointer = saved;
        auto failed = false;
        for (auto& element : signature.symbols) {
            if (pointer >= given.symbols.size()) { failed = true; break; }
            if (subexpression(element)) {
                auto& type = element.subexpression.type;
                flags.should_allow_undefined_signatures = type and expressions_match(*type, signature_type);                
                expression parsed_subexpression = csr(given, type, fdi, depth + 1, max_depth, pointer, stack, data, flags.not_at_top_level(), error);                
                if (parsed_subexpression.erroneous) {
                    pointer = saved;
                    if (subexpression(given.symbols[pointer])) {                                                
                        size_t local_pointer;
                        expression subexpression = {};
                        use_csr1(data, element, error, fdi, flags, given, local_pointer, pointer, stack, subexpression);                                                
                        if (subexpression.erroneous or local_pointer < given.symbols[pointer].subexpression.symbols.size()) { failed = true; break; }
                        solution.symbols.push_back({subexpression});
                        pointer++; continue;
                    } else { failed = true; break; }
                } solution.symbols.push_back({parsed_subexpression});
            } else if (element.identifier.name.value == given.symbols[pointer].identifier.name.value
                       and identifier(given.symbols[pointer]) and identifier(element)) {
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
    
    if (subexpression(given.symbols[pointer]) and contains_a_block_starting_from(pointer + 1, given.symbols))
        return parse_new_abstraction_definition(data, error, expected, flags, given, pointer, saved, stack);
        
    else if (flags.should_allow_undefined_signatures and pointer < given.symbols.size()) {
        fdi.symbols.push_back(given.symbols[pointer++]);
        return fdi;       //// ?      is this right?
    }
    return {true};
}

expression resolve(expression given, expression& expected_type, stack& stack, tu_data& data, flags flags, bool& error) {    
    std::sort(stack.back().begin(), stack.back().end(), [](auto a, auto b) { return a.symbols.size() > b.symbols.size(); });
    prune_extraneous_subexpressions(given);             
    expression solution = {}, fdi = {};
    size_t pointer;
    use_csr2(data, error, expected_type, fdi, flags, given, pointer, solution, stack);
    if (pointer < given.symbols.size() or not solution.type) solution.erroneous = true;
    if (not solution.erroneous and expressions_match(solution, unit_type)) solution.type = &unit_type;
    if (expressions_match(expected_type, infered_type) and solution.type) expected_type = *solution.type;
    return solution;
}

std::unique_ptr<llvm::Module> analyze(translation_unit unit, llvm::LLVMContext& context, struct file file) {
    
    srand((unsigned)time(nullptr));
    llvm::IRBuilder<> builder(context);
    auto module = llvm::make_unique<llvm::Module>(file.name, context);     
    auto triple = llvm::sys::getDefaultTargetTriple();    
    module->setTargetTriple(triple);    
    auto main_function = create_main(builder, context, module);
    
    static bool found_main = false;
    bool error = false;
    stack stack = {builtins};
    flags flags = {};
    tu_data data = {file, module.get(), main_function, builder};
    
    auto& body = unit.list.expressions;
    std::vector<expression> parsed_body = {};
    
    for (auto expression : body) {        
        auto solution = resolve(expression, unit_type, stack, data, flags
                            .dont_allow_undefined()
                            .generate_code().at_top_level()
                            .parsing_a_type(), error);
        
        if (solution.erroneous) error = true;
        else parsed_body.push_back(solution);
    }
    body = parsed_body;
    
    if (contains_top_level_statements(body)) {
        if (found_main) error = true;
        else { file.is_main = true; found_main = true; }
    } else if (found_main) main_function->eraseFromParent();    
    
    if (llvm::verifyFunction(*main_function)) append_return_0_statement(builder, context);     
    if (llvm::verifyModule(*module, &llvm::errs())) error = true;
    if (debug) print_translation_unit(unit, file);
    if (error) { throw "analysis error"; } 
    else { return module; }
}
