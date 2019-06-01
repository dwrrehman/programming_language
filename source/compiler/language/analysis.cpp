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
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
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


bool expressions_fmatch(expression first, expression second);

expression csrf(std::vector<std::vector<expression>>& stack, 
                const expression given, const size_t depth, 
                const size_t max_depth, size_t& pointer, 
                struct expression*& expected, 
                bool can_define_new_signature, 
                bool is_at_top_level, bool is_parsing_type, 
                llvm::Module* module, struct file file, 
                llvm::Function* function, llvm::IRBuilder<>& builder, 
                bool should_generate_code, expression& fdi);

bool adpf(abstraction_definition& given, 
          std::vector<std::vector<expression>>& stack, 
          llvm::Module* module, struct file file, llvm::Function* function, 
          llvm::IRBuilder<>& builder, bool should_generate_code);

expression resolvfe(std::vector<std::vector<expression>>& stack, 
                    expression given, expression& expected_solution_type, 
                    bool can_define_new_signature,
                    bool is_at_top_level, bool is_parsing_type, 
                    llvm::Module* module, struct file file, 
                    llvm::Function* function, llvm::IRBuilder<>& builder, 
                    bool should_generate_code);





static void parse_abstraction_body(abstraction_definition &given, stack &stack, data& data, flags flags, bool& error) {        
    add_signature_to_symbol_table(given.call_signature, stack);
    auto& body = given.body.list.expressions;    
    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size() - 1; i++) {
            
            auto type = unit_type;
            auto solution = res(body[i], type, stack, data, flags
                                .dont_allow_undefined()
                                .at_top_level()
                                .not_parsing_a_type(), error);     
            
            if (solution.erroneous) error = true;                
            else parsed_body.push_back(solution);
        }
        
        auto solution = res(body[body.size() - 1], given.return_type, stack, data, flags
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

static void parse_return_type(abstraction_definition &given, stack &stack, data& data, flags flags, bool& error) {
    
    if (given.return_type.symbols.size()) {
        auto type = infered_type;
        
        given.return_type = res(given.return_type, type, stack, data, flags
                                .allow_undefined()
                                .not_at_top_level()
                                .parsing_a_type(), error);
        if (given.return_type.erroneous) error = true;
        
        if (given.return_type.type
            and expressions_match(*given.return_type.type, unit_type) 
            and given.return_type.symbols.empty()) given.return_type = unit_type;
        else if (given.return_type.type 
                 and expressions_match(*given.return_type.type, none_type)) given.return_type = none_type;
    } else given.return_type = infered_type;
    
    given.call_signature.type = new expression();
    *given.call_signature.type = given.return_type;
    given.call_signature.type->was_allocated = true;
}

static void parse_signature(abstraction_definition &given, stack &stack, data& data, flags flags, bool& error) {
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
                while (pointer < sub.symbols.size()) definition.return_type.symbols.push_back(sub.symbols[pointer++]);                
                parse_signature(definition, stack, data, flags, error);
                parse_return_type(definition, stack, data, flags, error);
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
        } else error = true;        
    }
    given.call_signature = result;
}

void parse_abstraction(abstraction_definition& given, stack& stack, data& data, flags flags, bool& error) {
    clean(given.body);    
    stack.push_back(stack.back());
    parse_signature(given, stack, data, flags, error);
    parse_return_type(given, stack, data, flags, error);
    parse_abstraction_body(given, stack, data, flags, error);
    stack.pop_back();    
}

static abstraction_definition preliminary_parse_abstraction(const expression &given, size_t &pointer) {
    abstraction_definition definition = {};
    definition.call_signature = given.symbols[pointer++].subexpression;
    while (given.symbols[pointer].type != symbol_type::block) {
        definition.return_type.symbols.push_back(given.symbols[pointer++]);
    } definition.body = given.symbols[pointer++].block;
    return definition;
}

expression csr(               
               const expression& given,
               expression*& expected, 
               expression& fdi,
               
               const size_t depth, 
               const size_t max_depth,
               size_t& pointer,
               
               stack& stack,                 
               data& data,
               flags flags,
               bool& error
               ) {

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
        
    } else if (pointer < given.symbols.size() and given.symbols[pointer].type == symbol_type::llvm_literal) {
        auto llvm_string = given.symbols[pointer].llvm.literal.value;
        return parse_llvm_string(given, llvm_string, pointer, stack, data, flags, error);
    }

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
            if (element.type == symbol_type::subexpression) {
                auto& type = element.subexpression.type;
                flags.should_allow_undefined_signatures = type and expressions_match(*type, signature_type);                
                expression subexpression = csr(given, type, fdi, depth + 1, max_depth, pointer, stack, data, flags.not_at_top_level(), error);                
                if (subexpression.erroneous) {
                    pointer = saved;
                    if (given.symbols[pointer].type == symbol_type::subexpression) {
                        
                        ///////        this should be a seperate function: parse expression?
                        size_t local_pointer = 0, current_depth = 0;
                        expression subexpression = {};
                        while (current_depth <= max_expression_depth) {
                            local_pointer = 0;
                            auto& type = element.subexpression.type;
                            flags.should_allow_undefined_signatures = type and expressions_match(*type, signature_type);
                            subexpression = csr(given.symbols[pointer].subexpression,  type, fdi, 0, current_depth, local_pointer, stack, data, flags.not_at_top_level(), error);
                            if (subexpression.erroneous or local_pointer < given.symbols[pointer].subexpression.symbols.size()) {
                                current_depth++;
                            } else break;
                        }
                        ///////
                        
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
    } else if (flags.should_allow_undefined_signatures and pointer < given.symbols.size()) {
        fdi.symbols.push_back(given.symbols[pointer++]);
    }
    return {true};
}

expression res(
               expression given, 
               expression& expected_type,
               stack& stack,
               data& data,
               flags flags,
               bool& error
               ) {    
    std::sort(stack.back().begin(), stack.back().end(), [](auto a, auto b) { return a.symbols.size() > b.symbols.size(); });
    prune_extraneous_subexpressions(given);

    expression fdi = {};
    expression solution = {};
    size_t pointer = 0, max_depth = 0;
    while (max_depth <= max_expression_depth) {
        pointer = 0;
        auto copy = expected_type;
        solution.type = &copy;
        flags.should_allow_undefined_signatures = expressions_match(*solution.type, signature_type);        
        solution = csr(given, solution.type, fdi, 0, max_depth, pointer, stack, data, flags, error);
        if (solution.erroneous or pointer < given.symbols.size() or not solution.type) { max_depth++; }
        else break;
    }
    if (pointer < given.symbols.size() or not solution.type) solution.erroneous = true;
    if (not solution.erroneous and expressions_match(solution, unit_type)) solution.type = &unit_type;
    if (expressions_match(expected_type, infered_type) and solution.type) expected_type = *solution.type;
    return solution;
}

std::unique_ptr<llvm::Module> analyze(translation_unit unit, llvm::LLVMContext& context, struct file file) {
    
    srand((unsigned)time(nullptr));
    auto module = llvm::make_unique<llvm::Module>(file.name, context); 
    llvm::IRBuilder<> builder(context);
    
    static bool found_main = false;
    bool error = false;
    wrap_into_main(unit);
    stack stack = {builtins};

    auto type = type_type;
    auto& main = unit.list.expressions[0].symbols[0].abstraction;
    auto& body = main.body.list.expressions;
        
    auto triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(triple);
    
    std::vector<llvm::Type*> parameters = {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()};
    llvm::FunctionType* main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), parameters, false);
    llvm::Function* main_function = llvm::Function::Create(main_type, llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main_function));
    
    struct flags flags = {};    
    struct data data = {
        file, 
        module.get(), 
        main_function, 
        builder        
    };
    
    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size(); i++) {
            auto type = unit_type;
            auto solution = res(body[i], type, stack, data, flags
                                .dont_allow_undefined()
                                .generate_code().at_top_level()
                                .parsing_a_type(), error);
            
            if (solution.erroneous) error = true;                
            else parsed_body.push_back(solution);
        }
        main.body.list.expressions = parsed_body;
        if (contains_top_level_statements(parsed_body)) {
            if (found_main) error = true;
            else {
                file.is_main = true;
                found_main = true;
            }
        }
    }

    if (llvm::verifyFunction(*main_function)) {
        llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
        builder.CreateRet(value); 
    }

    if (llvm::verifyModule(*module, &llvm::errs())) error = true;             
    
    if (error) {
        std::cout << "\n\n\tCSR ERROR\n\n\n\n";
        throw "analysis error";
    } else {
        if (debug) std::cout << "\n\n\tsuccess.\n\n\n";
        return module;
    }
}
