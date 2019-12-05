//
//  helpers.hpp
//  language
//
//  Created by Daniel Rehman on 1908213.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef helpers_hpp
#define helpers_hpp

#include "analysis_ds.hpp"
#include "llvm/IR/Function.h"

bool subexpression(const symbol& s);
bool identifier(const symbol& s);
bool llvm_string(const symbol& s);

bool contains_final_terminator(llvm::Function* main_function);
void append_return_0_statement(llvm::IRBuilder<> &builder, llvm::Function* main_function, llvm::LLVMContext &context); 
void call_donothing(llvm::IRBuilder<> &builder, llvm_module& module);
llvm::Function* create_main(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm_module& module);

void prune_extraneous_subexpressions(expression_list& given);
resolved_expression_list resolve_expression_list(const expression_list& given, nat given_type, llvm::Function*& function, state& state);
resolved_expression resolve_expression(const expression& given, nat given_type, llvm::Function*& function, state& state);
resolved_expression resolve(const expression& given, nat given_type, llvm::Function*& function, nat& index, const nat depth, const nat max_depth, nat fdi_length, state& state);
std::string emit(const llvm_module& module);
void remove_extraneous_insertion_points_in(llvm_module& module);
void delete_empty_blocks(llvm_module& module);
void move_lone_terminators_into_previous_blocks(llvm_module& module); 

#endif /* helpers_hpp */
