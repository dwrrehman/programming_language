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
#include "parser.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/IRBuilder.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

////////////////// comparisons ///////////////////////////

bool expressions_match(expression first, expression second);
bool subexpression(const symbol& s);
bool identifier(const symbol& s);
bool are_equal_identifiers(const symbol &first, const symbol &second);
bool symbols_match(symbol first, symbol second);
bool expressions_match(expression first, expression second);

////////////////////////////////// General helpers ////////////////////////////////

std::string random_string();
void print(std::vector<std::string> v);
void prune_extraneous_subexpressions(expression& given);
std::vector<expression> filter_subexpressions(expression given);
void append_return_0_statement(llvm::IRBuilder<> &builder, llvm::LLVMContext &context) ;
void declare_donothing(llvm::IRBuilder<> &builder, const std::unique_ptr<llvm::Module> &module);
bool found_unit_value_expression(const expression &given);
expression parse_unit_expression(expression& given, size_t& index, state& state) ;
bool found_llvm_string(const expression &given, size_t &pointer) ;
llvm::Function* create_main(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, const std::unique_ptr<llvm::Module>& module);

void interpret_file_as_llvm_string(const struct file &file, state &state);
/////////////////////////////////////// PARSE LLVM STRING ///////////////////////////////////////////

llvm::Type* parse_llvm_string_as_type(std::string given, state& state, llvm::SMDiagnostic& errors) ;
bool parse_llvm_string_as_instruction(std::string given, llvm::Function* function, state& state, llvm::SMDiagnostic& errors);
bool parse_llvm_string_as_function(std::string given, state& state, llvm::SMDiagnostic& errors);
expression parse_llvm_string(const expression &given, std::string llvm_string, size_t &pointer, state& state, flags flags) ;

expression csr_single(expression given, size_t& index, const size_t depth, const size_t max_depth, state& state, flags flags); 
expression_list csr(expression_list given, size_t& index, const size_t depth, const size_t max_depth, state& state, flags flags); 
expression_list traverse(expression_list given, state& state, flags flags);
expression_list resolve(expression_list given, state& state, flags flags); // interface function:
#endif /* helpers_hpp */
