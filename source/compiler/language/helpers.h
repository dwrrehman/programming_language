//
//  helpers.h
//  language
//
//  Created by Daniel Rehman on 1905315.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef helpers_h
#define helpers_h

#include "analysis.hpp"
#include "parser.hpp"
#include "nodes.hpp"
#include "lists.hpp"
#include "builtins.hpp"
#include "debug.hpp"

#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
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
#include "llvm/Support/MemoryBuffer.h"
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

////////////////// comparisons ///////////////////////////

bool expressions_match(expression first, expression second);


bool subexpression(const symbol& s) {
    return s.type == symbol_type::subexpression;
}
bool identifier(const symbol& s) {
    return s.type == symbol_type::identifier;
}

bool are_equal_identifiers(const symbol &first, const symbol &second) {
    return first.type == symbol_type::identifier and second.type == symbol_type::identifier 
        and first.identifier.name.value == second.identifier.name.value;
}

bool symbols_match(symbol first, symbol second) {
    if (subexpression(first) and subexpression(second) and expressions_match(first.subexpression, second.subexpression)) return true;
    else if (are_equal_identifiers(first, second)) return true;
    else if (first.type == symbol_type::llvm_literal and second.type == symbol_type::llvm_literal) return true;
    else return false;
}

bool expressions_match(expression first, expression second) {
    if (first.symbols.size() != second.symbols.size()) return false;
    for (size_t i = 0; i < first.symbols.size(); i++) {
        if (not symbols_match(first.symbols[i], second.symbols[i])) return false;
    }
    if (first.erroneous or second.erroneous) return false;
    if (!first.type and !second.type) return true;
    
    if (first.llvm_type and second.llvm_type) {
        std::string first_llvm_type = "", second_llvm_type = "";
        first.llvm_type->print(llvm::raw_string_ostream(first_llvm_type) << "");
        second.llvm_type->print(llvm::raw_string_ostream(second_llvm_type) << "");
        return first_llvm_type == second_llvm_type;
    } else if (first.llvm_type or second.llvm_type) return false;
    
    else if (first.type and second.type and expressions_match(*first.type, *second.type)) return true;
    else return false;
}


























////////////////////////////////// General helpers ////////////////////////////////

std::string random_string() {
    static int num = 0;
    std::stringstream stream;
    stream << std::hex << rand();
    return std::string(stream.str()) + std::to_string(num++);
}

void clean(block& body) {
    block result = {};
    for (auto expression : body.list.expressions) 
        if (not expression.symbols.empty()) result.list.expressions.push_back(expression);
    body = result;
}

void prune_extraneous_subexpressions(expression& given) {
    while (given.symbols.size() == 1
           and subexpression(given.symbols[0])
           and given.symbols[0].subexpression.symbols.size()) {
        auto save = given.symbols[0].subexpression.symbols;
        given.symbols = save;
    }
    for (auto& symbol : given.symbols)
        if (subexpression(symbol)) prune_extraneous_subexpressions(symbol.subexpression);
}

std::vector<expression> filter_subexpressions(expression given) {
    std::vector<expression> subexpressions = {};    
    for (auto element : given.symbols) 
        if (subexpression(element)) subexpressions.push_back(element.subexpression);    
    return subexpressions;
}

abstraction_definition preliminary_parse_abstraction(const expression &given, size_t &pointer) {
    abstraction_definition definition = {};
    definition.call_signature = given.symbols[pointer++].subexpression;
    while (given.symbols[pointer].type != symbol_type::block) {
        definition.return_type.symbols.push_back(given.symbols[pointer++]);
    } definition.body = given.symbols[pointer++].block;
    return definition;
}

expression* generate_abstraction_type_for(abstraction_definition def) {
    auto type = new expression();
    type->was_allocated = true;
    auto parameter_list = filter_subexpressions(def.call_signature);
    if (parameter_list.empty()) {
        *type = def.return_type;
        return type;
    }
    for (auto parameter : parameter_list) {
        expression t = type_type;
        if (parameter.type) t = *parameter.type;
        type->symbols.push_back(t);
    }
    type->symbols.push_back({def.return_type});
    type->type = &type_type;
    return type;
}

bool contains_a_block_starting_from(size_t begin, std::vector<symbol> list) {
    for (; begin < list.size(); begin++)
        if (list[begin].type == symbol_type::block) return true;
    return false;
}

bool contains_top_level_statements(std::vector<expression> list) {
    for (auto e : list) if (not (e.symbols.size() == 1 and e.symbols[0].type == symbol_type::abstraction_definition)) return true;
    return false;
}

void append_return_0_statement(llvm::IRBuilder<> &builder, llvm::LLVMContext &context) {
    llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    builder.CreateRet(value);
}

bool found_unit_expression(const expression &given) {
    return given.symbols.empty() or (given.symbols.size() == 1 and subexpression(given.symbols[0]) and given.symbols[0].subexpression.symbols.empty());
}

expression parse_unit_expression(expression *&expected, const expression &given, size_t &pointer) {
    if (given.symbols.size() == 1
        and subexpression(given.symbols[0])
        and given.symbols[0].subexpression.symbols.empty()) pointer++;
    if (expressions_match(*expected, infered_type)) expected = &unit_type;
    if (expressions_match(*expected, unit_type)) return unit_type;
    else return {true};
}

bool found_llvm_string(const expression &given, size_t &pointer) {
    return pointer < given.symbols.size() and given.symbols[pointer].type == symbol_type::llvm_literal;
}

llvm::Function *create_main(llvm::IRBuilder<> &builder, llvm::LLVMContext &context, const std::unique_ptr<llvm::Module, std::default_delete<llvm::Module> > &module) {
    std::vector<llvm::Type*> parameters = {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()};
    llvm::FunctionType* main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), parameters, false);
    llvm::Function* main_function = llvm::Function::Create(main_type, llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main_function));
    return main_function;
}




























//////////////////////////// SYMBOL TABLE CONVERTERS ./////////////////////////////



std::string expression_to_string(expression given) {
    std::string result = "(";
    size_t i = 0;
    for (auto symbol : given.symbols) {
        if (symbol.type == symbol_type::identifier) result += symbol.identifier.name.value;
        else if (subexpression(symbol)) {
            result += "(" + expression_to_string(symbol.subexpression) + ")";
        }
        if (i < given.symbols.size() - 1) result += " ";
        i++;
    }
    result += ")";
    if (given.llvm_type) {
        std::string type = "";
        given.llvm_type->print(llvm::raw_string_ostream(type) << "");
        result += " " + type;
    } else if (given.type) {
        result += " " + expression_to_string(*given.type);
    }
    return result;
}


expression string_to_expression_tail(std::vector<expression> list, stack& stack, tu_data& data, flags flags, bool& error) {
    if (list.empty()) return {};
    if (list.size() == 1 and expressions_match(list.back(), type_type)) return type_type;          
    auto signature = list.front();
    
    for (auto& element : signature.symbols) {
        if (subexpression(element)) {
            auto subexpressions = filter_subexpressions(element.subexpression); 
            
            auto result = string_to_expression_tail(subexpressions, stack, data, flags, error);
            if (result.erroneous) error = true;
            else element.subexpression = result;
        }
    }
    list.erase(list.begin());
    auto type = string_to_expression_tail(list, stack, data, flags
                                          .dont_allow_undefined()
                                          .not_at_top_level()
                                          .not_parsing_a_type(), error);
    if (signature.symbols.empty() and expressions_match(type, type_type)) return unit_type;    
    auto result = resolve(signature, type, stack, data, flags, error);    
    if (result.erroneous) error = true;    
    return result;
}

expression string_to_expression(std::string given, stack& stack, tu_data& data, flags flags, bool& error) {
    struct file file = {"<llvm string symbol>", given};
    start_lex(file);
    return string_to_expression_tail(filter_subexpressions(parse_expression(file, false, false)), stack, data, flags, error);
}



























/////////////////////////////////////// PARSE LLVM STRING ///////////////////////////////////////////


llvm::Type* parse_llvm_string_as_type(std::string given, stack& stack, tu_data& data, llvm::SMDiagnostic& errors) {
    return llvm::parseType(given, errors, *data.module);
}

bool parse_llvm_string_as_instruction(std::string given, stack& stack, tu_data& data, llvm::SMDiagnostic& errors) {
    
    std::string body = "";
    data.function->print(llvm::raw_string_ostream(body) << "");
    
    const size_t bb_count = data.function->getBasicBlockList().size();
    
    body.pop_back(); // delete the newline;
    body.pop_back(); // delete the close brace.
    body += given + "\nunreachable\n}\n";
    
    const std::string current_name = data.function->getName();
    data.function->setName("_anonymous_" + random_string());
    
    llvm::MemoryBufferRef reference(body, "<llvm-string>");
    llvm::ModuleSummaryIndex my_index(true);
    
    if (llvm::parseAssemblyInto(reference, data.module, &my_index, errors)) {
        data.function->setName(current_name);
        return false;
    } else {
        auto& made = data.module->getFunctionList().back();
        made.getBasicBlockList().back().back().eraseFromParent();
        if (bb_count != made.getBasicBlockList().size()) made.getBasicBlockList().back().eraseFromParent();
        data.function->getBasicBlockList().clear();
        data.builder.SetInsertPoint(llvm::BasicBlock::Create(data.module->getContext(), "entry", data.function));
        data.builder.CreateUnreachable();
        auto& insert_before_point = data.function->getBasicBlockList().back().back();
        for (auto& bb : made.getBasicBlockList()) {
            llvm::ValueToValueMapTy vmap;
            for (auto& inst: bb.getInstList()) {
                auto* new_inst = inst.clone();
                new_inst->setName(inst.getName());
                new_inst->insertBefore(&insert_before_point);
                vmap[&inst] = new_inst;
                llvm::RemapInstruction(new_inst, vmap, llvm::RF_NoModuleLevelChanges | llvm::RF_IgnoreMissingLocals);
            }
        }
        data.function->getBasicBlockList().back().back().eraseFromParent();      // delete the trailing unreachable.
        made.eraseFromParent();
        data.function->setName(current_name);
        return true;
    }
}

bool parse_llvm_string_as_function(std::string given, stack& stack, tu_data& data, llvm::SMDiagnostic& errors) {
    llvm::MemoryBufferRef reference(given, "<llvm-string>");
    llvm::ModuleSummaryIndex my_index(true);
    return !llvm::parseAssemblyInto(reference, data.module, &my_index, errors);        
}

static expression parse_llvm_string(
                                    const expression &given, std::string llvm_string, size_t &pointer,
                                    stack& stack, tu_data& data, flags flags, bool& error                                    
                                    ) {
    
    if (flags.is_at_top_level and not flags.is_parsing_type) {
        
        llvm::SMDiagnostic instruction_errors;
        llvm::SMDiagnostic function_errors;
        
        if (parse_llvm_string_as_function(llvm_string, stack, data, function_errors)) {
            expression solution = {};
            solution.erroneous = false;
            solution.type = &unit_type;
            solution.symbols = {};
            symbol s = {};
            s.type = symbol_type::llvm_literal;
            s.llvm = given.symbols[pointer++].llvm;
            solution.symbols.push_back(s);
            return solution;
            
        } else if (parse_llvm_string_as_instruction(llvm_string, stack, data, instruction_errors)) {
            expression solution = {};
            solution.erroneous = false;
            solution.type = &unit_type;
            solution.symbols = {};
            symbol s = {};
            s.type = symbol_type::llvm_literal;
            s.llvm = given.symbols[pointer++].llvm;
            solution.symbols.push_back(s);
            return solution;
            
        } else {
            std::cout << "ins: llvm: "; // TODO: make this have color!
            instruction_errors.print(data.file.name.c_str(), llvm::errs()); // temp
            std::cout << "func: llvm: "; // TODO: make this have color!
            function_errors.print(data.file.name.c_str(), llvm::errs());
            return {true};
        }
        
    } else if (flags.is_parsing_type and not flags.is_at_top_level) {
        
        llvm::SMDiagnostic type_errors;
        
        if (auto llvm_type = parse_llvm_string_as_type(llvm_string, stack, data, type_errors)) {
            
            expression solution = {};
            solution.erroneous = false;
            solution.llvm_type = llvm_type;
            solution.type = &type_type;
            solution.symbols = {};
            symbol s = {};
            s.type = symbol_type::llvm_literal;
            s.llvm = given.symbols[pointer++].llvm;
            solution.symbols.push_back(s);
            return solution;
            
        } else {
            std::cout << "llvm: "; // TODO: make this have color!
            type_errors.print(data.file.name.c_str(), llvm::errs()); // temp, see above block comment.
            return {true};
        }
    } else {        
        return {true};
    }
}

#endif /* helpers_h */
