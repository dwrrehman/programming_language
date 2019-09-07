//
//  analysis_ds.hpp
//  language
//
//  Created by Daniel Rehman on 1908213.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef analysis_ds_hpp
#define analysis_ds_hpp

#include "arguments.hpp"
#include "nodes.hpp"
#include "llvm/IR/IRBuilder.h"
#include <iostream>

struct file_data {
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


class symbol_table;

struct state {
    symbol_table& stack;     
    file_data& data;
    bool& error;
};


#endif /* analysis_ds_hpp */
