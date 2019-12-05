#ifndef helpers_hpp
#define helpers_hpp

#include "analysis_ds.hpp"
#include "llvm/IR/Function.h"

bool subexpression(const symbol& s);
bool contains_final_terminator(llvm::Function* main_function);
resolved_expression_list resolve_expression_list(const expression_list& given, nat given_type, llvm::Function*& function, state& state);
resolved_expression resolve_expression(const expression& given, nat given_type, llvm::Function*& function, state& state);
resolved_expression resolve(const expression& given, nat given_type, llvm::Function*& function, nat& index, const nat depth, const nat max_depth, nat fdi_length, state& state);
void remove_extraneous_insertion_points_in(llvm_module& module);
void delete_empty_blocks(llvm_module& module);
void move_lone_terminators_into_previous_blocks(llvm_module& module); 

#endif /* helpers_hpp */
