//
//  helpers.cpp
//  language
//
//  Created by Daniel Rehman on 1908213.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "helpers.hpp"

#include "analysis_ds.hpp"
#include "compiler.hpp"
#include "parser.hpp"
#include "builtins.hpp"
#include "symbol_table.hpp"
#include "lists.hpp"
#include "error.hpp"
#include  "llvm_parser.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/daniels_interpreter/MCJIT.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/GenericValue.h"

#include <cstdlib>
#include <iostream>
#include <sstream>


const resolved_expression resolution_failure = {0, {}, true};

bool subexpression(const symbol& s) { return s.type == symbol_type::subexpression; }
bool identifier(const symbol& s) { return s.type == symbol_type::identifier; }
bool llvm_string(const symbol& s) { return s.type == symbol_type::llvm_literal; }
bool parameter(const symbol &symbol) { return subexpression(symbol); }

bool are_equal_identifiers(const symbol &first, const symbol &second) {
    return identifier(first) and identifier(second) 
    and first.identifier.name.value == second.identifier.name.value;
}

bool expressions_match(expression first, expression second);

bool symbols_match(symbol first, symbol second) {
//    if (subexpression(first) and subexpression(second) 
//        and expressions_match(first.expressions..back(), 
//                              second.expressions..back())) return true;    ///TODO: this code is suspicious, alot.
//    else if (are_equal_identifiers(first, second)) return true;
//    else if (first.type == symbol_type::llvm_literal and second.type == symbol_type::llvm_literal) return true;
//    else return false;
    return false;
}

bool expressions_match(expression first, expression second) {
    if (first.error or second.error) return false;    
    if (first.symbols.size() != second.symbols.size()) return false;
    for (size_t i = 0; i < first.symbols.size(); i++) 
        if (not symbols_match(first.symbols[i], second.symbols[i])) return false;    
    if (!first.type and !second.type) return true;
    
    if (first.llvm_type and second.llvm_type) {
        std::string first_llvm_type = "", second_llvm_type = "";
        first.llvm_type->print(llvm::raw_string_ostream(first_llvm_type) << "");
        second.llvm_type->print(llvm::raw_string_ostream(second_llvm_type) << "");
        return first_llvm_type == second_llvm_type;
    } else if (first.llvm_type or second.llvm_type) return false;
    
    else if (first.type == second.type) return true;
    else return false;
}






void append_return_0_statement(llvm::IRBuilder<> &builder, llvm::LLVMContext &context) {
    llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    builder.CreateRet(value);
}

void call_donothing(llvm::IRBuilder<> &builder, const std::unique_ptr<llvm::Module> &module) {
    llvm::Function* donothing = llvm::Intrinsic::getDeclaration(module.get(), llvm::Intrinsic::donothing);        
    builder.CreateCall(donothing);
}

llvm::Function* create_main(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, const std::unique_ptr<llvm::Module>& module) {
    std::vector<llvm::Type*> state = {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()};
    llvm::FunctionType* main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), state, false);
    llvm::Function* main_function = llvm::Function::Create(main_type, llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main_function));    
    return main_function;
}




static std::vector<llvm::GenericValue> turn_into_value_array(std::vector<nat> canonical_arguments, std::unique_ptr<llvm::Module>& module) {
    std::vector<llvm::GenericValue> arguments = {};
    for (auto index : canonical_arguments) 
        arguments.push_back(llvm::GenericValue {llvm::ConstantInt::get(llvm::Type::getInt32Ty(module->getContext()), index)});
    return arguments;
}

nat evaluate(std::unique_ptr<llvm::Module>& module, llvm::Function* function, std::vector<nat> args) {    
    set_data_layout(module);
    auto jit = llvm::EngineBuilder(std::unique_ptr<llvm::Module>{module.get()}).setEngineKind(llvm::EngineKind::JIT).create();
    jit->finalizeObject();
    return jit->runFunction(function, turn_into_value_array(args, module)).IntVal.getLimitedValue();
}





bool matches(expression given, expression signature, std::vector<resolved_expression_list>& args, llvm::Function*& function, nat& index, const nat depth, const nat max_depth, state& state, flags flags) {
    if (given.type != signature.type and given.type != intrin::infered) return false;
    for (auto symbol : signature.symbols) {
                
        if (parameter(symbol) and subexpression(given.symbols[index])) {
            auto argument = resolve_expression_list(given.symbols[index].expressions, function, state, flags);
            args.push_back(argument);            
            if (argument.error) return false;
            index++;
            
        } else if (parameter(symbol)) {            
            auto argument = resolve(given, function, index, depth + 1, max_depth, state, flags);            
            if (argument.error) return false;
            args.push_back(resolved_expression_list {std::vector<resolved_expression> {argument}});                                    
            
        } else if (not are_equal_identifiers(symbol, given.symbols[index])) {
            return false;
        } else {            
            index++;
        }
    }
    return true;
}

resolved_expression resolve(
                               expression given, llvm::Function*& function, 
                               nat& index, nat depth, nat max_depth, 
                               state& state, flags flags
                               ) {
    
    if (index >= given.symbols.size() or not given.type or depth > max_depth) return resolution_failure;
    
    if (llvm_string(given.symbols[index]))         
        return parse_llvm_string(given, function, given.symbols[index].llvm.literal.value, index, state, flags);
    
    size_t saved = index;
    for (auto signature_index : state.stack.top()) {
        index = saved;        
        std::vector<resolved_expression_list> args = {};
        if (matches(given, state.stack.get(signature_index), args, function, index, depth, max_depth, state, flags)) 
            return {signature_index, args, false};
    }
    
    return resolution_failure;
}

resolved_expression resolve_expression(expression given, llvm::Function*& function, state& state, flags flags) {
    resolved_expression solution = {};
    for (nat max_depth = 0; max_depth <= max_expression_depth; max_depth++) {            
        nat pointer = 0;
        solution = resolve(given, function, pointer, 0, max_depth, state, flags);
        if (not solution.error and pointer == given.symbols.size()) break;
    }
    return solution;
}

resolved_expression_list resolve_expression_list(expression_list given, llvm::Function*& function, state& state, flags flags) {
    /// this is the function we should target in order to implement the last expression being of type <return type / parent exprs type>
    resolved_expression_list solutions {};
    for (auto expression : given.list)
        solutions.list.push_back(resolve_expression(expression, function, state, flags));
    return solutions;
}
