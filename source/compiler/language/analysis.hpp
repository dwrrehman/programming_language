//
//  analysis.hpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef analysis_hpp
#define analysis_hpp

#include "arguments.hpp"
#include "nodes.hpp"
#include "symbol_table.hpp"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/IRBuilder.h"
#include <iostream>

struct translation_unit_data {
    struct file file;
    llvm::Module* module;
    llvm::IRBuilder<>& builder;
};

struct flags {
    bool should_allow_undefined_signatures = false;    
    bool should_generate_code = false;
    bool is_at_top_level = false; 
    bool is_parsing_type = false;
    
    struct flags& allow_undefined() { should_allow_undefined_signatures = true; return *this; }
    struct flags& dont_allow_undefined() { should_allow_undefined_signatures = false; return *this; }    
    struct flags& generate_code() { should_generate_code = true; return *this; }
    struct flags& dont_generate_code() { should_generate_code = false; return *this; }    
    struct flags& at_top_level() { is_at_top_level = true; return *this; }
    struct flags& not_at_top_level() { is_at_top_level = false; return *this; }    
    struct flags& parsing_a_type() { is_parsing_type = true; return *this; }    
    struct flags& not_parsing_a_type() { is_parsing_type = false; return *this; }
    
    flags(bool given_should_allow_undefined_signatures = false,
          bool given_should_generate_code = false, 
          bool given_is_at_top_level = false,
          bool given_is_parsing_type = false
          ):  
    should_allow_undefined_signatures(given_should_allow_undefined_signatures), 
    should_generate_code(given_should_generate_code), 
    is_at_top_level(given_is_at_top_level), 
    is_parsing_type(given_is_parsing_type) {}    
}; 

struct state {
    symbol_table& stack;    
    translation_unit_data& data;
    bool& error;
};

expression csr(expression given, size_t& index, const size_t depth, const size_t max_depth, state& state, flags flags); 
void adp(abstraction_definition& given, state& state, flags flags);
expression res(expression given, state& state, flags flags);

std::unique_ptr<llvm::Module> analyze(translation_unit unit, llvm::LLVMContext& context, struct file file);

#endif /* analysis_hpp */
