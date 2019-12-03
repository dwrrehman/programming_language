//
//  helpers.cpp
//  language
//
//  Created by Daniel Rehman on 1908213.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "helpers.hpp"

//#include "analysis_ds.hpp"
#include "compiler.hpp"
//#include "parser.hpp"
#include "builtins.hpp"
#include "symbol_table.hpp"
//#include "lists.hpp"
#include "error.hpp"
#include "llvm_parser.hpp"

//#include "llvm/AsmParser/Parser.h"
//#include "llvm/IR/ModuleSummaryIndex.h"
//#include "llvm/Support/SourceMgr.h"
//#include "llvm/IR/ValueSymbolTable.h"
//#include "llvm/Transforms/Utils/ValueMapper.h"
//#include "llvm/Transforms/Utils/Cloning.h"
//
//#include "llvm/Support/TargetRegistry.h"
//#include "llvm/Support/TargetSelect.h"

#include "llvm/ExecutionEngine/MCJIT.h"

#include "llvm/ExecutionEngine/GenericValue.h"
//
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

const resolved_expression resolution_failure = {0, {}, true};
const resolved_expression resolved_unit_value = {intrin::empty, {}, false};

void prune_extraneous_subexpressions(expression_list& given);
void prune_extraneous_subexpressions_in_expression(expression& given);

bool subexpression(const symbol& s) { return s.type == symbol_type::subexpression; }
bool identifier(const symbol& s) { return s.type == symbol_type::identifier; }
bool llvm_string(const symbol& s) { return s.type == symbol_type::llvm_literal; }
bool string_literal(const symbol& s) { return s.type == symbol_type::string_literal; }
bool parameter(const symbol &symbol) { return subexpression(symbol); }


bool contains_final_terminator(llvm::Function* main_function) {
    auto& blocks = main_function->getBasicBlockList();
    if (blocks.size()) {
        auto& instructions = blocks.back().getInstList();
        auto& last = instructions.back();
        if (last.isTerminator()) return true;
    }
    return false;
}

void append_return_0_statement(llvm::IRBuilder<> &builder, llvm::Function* main_function, llvm::LLVMContext& context) {
    llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);    
    builder.SetInsertPoint(&main_function->getBasicBlockList().back());
    builder.CreateRet(value);
}

void call_donothing(llvm::IRBuilder<> &builder, llvm_module& module) {
    llvm::Function* donothing = llvm::Intrinsic::getDeclaration(module.get(), llvm::Intrinsic::donothing);        
    builder.CreateCall(donothing);
}

static bool is_donothing_call(llvm::Instruction* ins) {   
    return ins 
        and std::string(ins->getOpcodeName()) == "call" 
        and std::string(ins->getOperand(0)->getName()) == "llvm.donothing";
}

static bool is_unreachable_instruction(llvm::Instruction* ins) {
    return ins 
        and std::string(ins->getOpcodeName()) == "unreachable";
}

llvm::Function* create_main(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm_module& module) {
    std::vector<llvm::Type*> state = {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()};
    auto main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), state, false);
    auto main_function = llvm::Function::Create(main_type, llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main_function));    
    return main_function;
}

void prune_extraneous_subexpressions_in_expression(expression& given) { // unimplemented
    while (given.symbols.size() == 1
           and subexpression(given.symbols[0])
           and given.symbols[0].expressions.list.size() == 1
           and given.symbols[0].expressions.list.back().symbols.size()) {
        auto save = given.symbols[0].expressions.list.back().symbols;
        given.symbols = save;
    }
    for (auto& symbol : given.symbols)
        if (subexpression(symbol)) prune_extraneous_subexpressions(symbol.expressions);
}

void prune_extraneous_subexpressions(expression_list& given) {
    for (auto& expression : given.list)
        prune_extraneous_subexpressions_in_expression(expression);
}

llvm::Constant* create_global_constant_string(llvm::Module* module, const std::string& string) {
    auto type = llvm::ArrayType::get(llvm::Type::getInt8Ty(module->getContext()), string.size() + 1);
    std::vector<llvm::Constant*> characters (string.size() + 1);        
    for (nat i = 0; i < string.size(); i++) characters[i] = llvm::ConstantInt::get(llvm::Type::getInt8Ty(module->getContext()), string[i]);    
    characters[string.size()] = llvm::ConstantInt::get(llvm::Type::getInt8Ty(module->getContext()), '\0');
    auto llvm_string = new llvm::GlobalVariable(*module, type, true, llvm::GlobalVariable::ExternalLinkage, 
                                                llvm::ConstantArray::get(type, characters), "string");    
    return llvm::ConstantExpr::getBitCast(llvm_string, llvm::Type::getInt8Ty(module->getContext())->getPointerTo());
}

static std::vector<llvm::GenericValue> turn_into_value_array(const std::vector<nat>& canonical_arguments, std::unique_ptr<llvm::Module>& module) {
    std::vector<llvm::GenericValue> arguments = {};
    for (auto index : canonical_arguments) 
        arguments.push_back(llvm::GenericValue {llvm::ConstantInt::get(llvm::Type::getInt32Ty(module->getContext()), index)});
    return arguments;
}

std::string emit(const llvm_module& module) {
    std::string string = "";
    module->print(llvm::raw_string_ostream(string) << "", NULL); 
    return string;
}

nat evaluate(llvm_module& module, llvm::Function* function, const std::vector<nat>& args) {    
    set_data_layout(module);
    auto jit = llvm::EngineBuilder(llvm_module{module.get()}).setEngineKind(llvm::EngineKind::JIT).create();
    jit->finalizeObject();
    return jit->runFunction(function, turn_into_value_array(args, module)).IntVal.getLimitedValue();
}

bool are_equal_identifiers(const symbol &first, const symbol &second) {
    return identifier(first) and identifier(second) 
    and first.identifier.name.value == second.identifier.name.value;
}

static std::vector<nat> generate_type_list(const expression_list &given, nat given_type) {
    if (given.list.empty()) return {};
    std::vector<nat> types (given.list.size() - 1, intrin::unit);
    types.push_back(given_type);
    return types;
}

static bool is_unit_value(const expression& expression) {
    return expression.symbols.size() == 1 
    and subexpression(expression.symbols[0]) 
    and expression.symbols[0].expressions.list.empty();
}   

static resolved_expression construct_signature(nat fdi_length, const expression& given, nat& index) {
    resolved_expression result = {intrin::typeless};
    result.signature = std::vector<symbol>(given.symbols.begin() + index, given.symbols.begin() + index + fdi_length);
    index += fdi_length;
    return result;
}

static bool matches(const expression& given, const expression& signature, nat given_type, std::vector<resolved_expression_list>& args, llvm::Function*& function, 
                    nat& index, nat depth, nat max_depth, nat fdi_length, state& state) {
        
    if (given_type != signature.type and given.type != intrin::infered) return false;
    for (auto symbol : signature.symbols) {
        if (index >= given.symbols.size()) return false;
        if (parameter(symbol) and subexpression(given.symbols[index])) {
            auto argument = resolve_expression_list(given.symbols[index].expressions, symbol.expressions.list.back().type, function, state);            
            if (argument.error) return false;
            args.push_back(argument);
            index++;
            
        } else if (parameter(symbol)) {
            auto argument = resolve(given, symbol.expressions.list.back().type, function, index, depth + 1, max_depth, fdi_length, state);
            if (argument.error) return false;
            args.push_back({{argument}});
            
        } else if (not are_equal_identifiers(symbol, given.symbols[index])) return false;
        else index++;
    }
    return true;
}

static resolved_expression parse_string(const expression &given, nat &index, state &state) {
    auto string_type = llvm::Type::getInt8PtrTy(state.data.module->getContext());
    auto actual_type = state.stack.master[intrin::llvm].llvm_type;
    
    if (actual_type == string_type or true ) {   ///TODO: delete the "or true".     (why is it even here?)
        resolved_expression string {};
        string.index = intrin::llvm;
        string.constant = create_global_constant_string(state.data.module, given.symbols[index].string.literal.value);
        return string;
    } else return resolution_failure;
}

resolved_expression resolve(const expression& given, nat given_type, llvm::Function*& function, 
                            nat& index, nat depth, nat max_depth, nat fdi_length,
                            state& state) {
    
    if (index >= given.symbols.size() or not given_type or depth > max_depth) 
        return resolution_failure;
    
    else if (given_type == intrin::abstraction)
        return construct_signature(fdi_length, given, index); 
    ///TODO: known bug: passing multiple different length signatures doesnt work with the current fdi solution.
    
    else if (llvm_string(given.symbols[index]) and given_type == intrin::unit) 
        return parse_llvm_string(function, given.symbols[index].llvm.literal.value, index, state);
    
    else if (string_literal(given.symbols[index]) and given_type == intrin::llvm) 
        parse_string(given, index, state);
    
    
    nat saved = index;
    for (auto signature_index : state.stack.top()) {        
        index = saved;
        std::vector<resolved_expression_list> args = {};                                
        if (matches(given, state.stack.get(signature_index), given_type, args, function, index, depth, max_depth, fdi_length, state))            
            return {signature_index, args, false};
    }    
    return resolution_failure;
}



///TODO: move this into error.cpp
static void print_unresolved_error(const expression &given, state &state) {
    const std::string name = expression_to_string(given, state.stack);
    print_error_message(state.data.file.name, "unresolved expression: " + name, given.starting_token.line, given.starting_token.column);
    print_source_code(state.data.file.text, {given.starting_token});
}

resolved_expression resolve_expression(const expression& given, nat given_type, llvm::Function*& function, state& state) {    
    
    if (is_unit_value(given) and given_type == intrin::unit) return resolved_unit_value;
    
    resolved_expression solution {};
    nat pointer = 0;
    for (nat max_depth = 0; max_depth <= max_expression_depth; max_depth++) {
        for (nat fdi_length = given.symbols.size(); fdi_length--;) {
            pointer = 0;
            solution = resolve(given, given_type, function, pointer, 0, max_depth, fdi_length, state);
            if (not solution.error and pointer == given.symbols.size()) break;
        }
        if (not solution.error and pointer == given.symbols.size()) break;
    }
    if (pointer < given.symbols.size()) solution.error = true; 
    if (solution.error) print_unresolved_error(given, state);
    return solution;
}

resolved_expression_list resolve_expression_list(const expression_list& given, nat given_type, llvm::Function*& function, state& state) {    
    if (given.list.empty()) return {{resolved_unit_value}, given_type != intrin::unit};
    nat i = 0;
    auto types = generate_type_list(given, given_type);    
    resolved_expression_list solutions {};
    for (auto expression : given.list) 
        solutions.list.push_back(resolve_expression(expression, types[i++], function, state));        
    for (auto e : solutions.list) solutions.error |= e.error;    
    return solutions;
}

void delete_empty_blocks(llvm_module& module) { 
    for (auto& function : module->getFunctionList()) {        
        llvm::SmallVector<llvm::BasicBlock*, 100> blocks = {};        
        for (auto& block : function.getBasicBlockList()) {
            if (block.empty()) blocks.push_back(&block);
        }
        llvm::DeleteDeadBlocks(blocks);
    }
}

void move_lone_terminators_into_previous_blocks(llvm_module& module) {
    for (auto& function : module->getFunctionList()) {        
        llvm::BasicBlock* previous = nullptr;
        
        for (auto& block : function.getBasicBlockList()) {            
            auto& instructions = block.getInstList();
            
            if (previous and instructions.size() == 1 
                and instructions.back().isTerminator())
                instructions.back().moveAfter(&previous->getInstList().back());
            
            previous = &block;
        }
    }
    
    ///KNOWN BUG: when we have no terminators in sight, this function 
    /// does remove the unneccessary basic block which is put between the 
    /// bits of code which should be in the same block.
    
}

void remove_extraneous_insertion_points_in(llvm_module& module) {    
    for (auto& function : module->getFunctionList()) {
        for (auto& block : function.getBasicBlockList()) {    
            auto terminator = block.getTerminator();
            if (!terminator) continue;
            auto previous = terminator->getPrevNonDebugInstruction();                        
            if (is_unreachable_instruction(terminator) and is_donothing_call(previous)) {
                previous->eraseFromParent();                 
                terminator->eraseFromParent();
            }
        }
    }    
}
