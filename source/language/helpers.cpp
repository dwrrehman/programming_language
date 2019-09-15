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

bool subexpression(const symbol& s) { return s.type == symbol_type::subexpression; }
bool identifier(const symbol& s) { return s.type == symbol_type::identifier; }
bool llvm_string(const symbol& s) { return s.type == symbol_type::llvm_literal; }

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


bool matches(expression given, llvm::Function*& function, expression& signature, nat& index, const nat depth, const nat max_depth, state& state, flags flags) {
//    if (given.type != signature.type and given.type != intrin::infered) return false;
//    for (auto& symbol : signature.symbols) {        
//        if (subexpression(symbol) and subexpression(given.symbols[index])) {
//            symbol.expressions = traverse(given.symbols[index].expressions, function, state, flags);
//            if (symbol.expressions.error) return false;
//            index++;
//        } else if (subexpression(symbol)) {
//            //symbol.expressions = csr(given, index, depth + 1, max_depth, state, flags);       /// i think this should be csr_single().
//            if (symbol.expressions.error) return false;            
//        } else if (not are_equal_identifiers(symbol, given.symbols[index])) return false;
//        else index++;
//    } return true;
    return {};
}

resolved_expression csr_single(expression given, llvm::Function*& function, nat& index, const nat depth, const nat max_depth, state& state, flags flags) {
//    if (index >= given.symbols.size() or not given.type or depth > max_depth) return failure; 
//    if (llvm_string(given.symbols[index]))         
//        return parse_llvm_string(given, function, given.symbols[index].llvm.literal.value, index, state, flags);
//    
//    size_t saved = index;
//    for (auto signature_index : state.stack.top()) {
//        index = saved;
//        auto signature = state.stack.get(signature_index);
//        if (matches(given, function, signature, index, depth, max_depth, state, flags)) return signature;
//    }
//    return failure;
    return {};
}

resolved_expression_list csr(expression_list given, llvm::Function*& function,nat& index, const nat depth, const nat max_depth, state& state, flags flags) {
//    for (auto& e : given.list)
//        e = csr_single(e, function, index, depth, max_depth, state, flags);
//    return given;
    return {};
}

resolved_expression_list traverse(expression_list given, llvm::Function*& function, state& state, flags flags) {
//    expression_list solution {};
//    for (size_t max_depth = 0; max_depth <= max_expression_depth; max_depth++) {
//        size_t pointer = 0;
//        solution = csr(given, function, pointer, 0, max_depth, state, flags); 
//        //if (not solution.error and pointer == given.symbols.size()) break;                   // i think we need to look through all expressions, and see if any have an error.
//    }
//    return solution;
    return {};
}


static void prepare_expressions(expression_list& given) {
//    for (auto& e : given.list) 
//        e.type = intrin::unit;
}

resolved_expression_list resolve(expression_list given, llvm::Function*& function, state& state, flags flags) {         
//    prepare_expressions(given);
//    auto saved = given;
//    given = traverse(given, function, state, flags);
//    
//    state.error = state.error or given.error;     
//    if (given.error) 
//        for (auto& e : saved.list) 
//            print_error_message(state.data.file.name, "could not resolve expression: \n\n" + expression_to_string(e, state.stack) + "\n\n", 0, 0);
//    
    return {};
}

