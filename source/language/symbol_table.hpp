//
//  symbol_table.hpp
//  language
//
//  Created by Daniel Rehman on 1908191.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef symbol_table_hpp
#define symbol_table_hpp

//#include "arguments.hpp"

//#include "converters.hpp"
//#include "debug.hpp"
//#include "helpers.hpp"
//#include "error.hpp"
//#include "builtins.hpp"

#include "lists.hpp" // nat 
#include "nodes.hpp"
#include "analysis_ds.hpp" // program_data

#include "llvm/IR/Function.h"

#include <vector>


//#include "llvm/IR/ValueSymbolTable.h"
//#include "llvm/IR/IRBuilder.h"


//
//#include <iostream>
//#include <iomanip>
//#include <algorithm>


struct stack_frame {
    std::vector<nat> indicies = {};  
};

struct signature_entry {
    expression signature = {};
    expression_list definition = {}; 
    nat precedence = 0;
    nat parent = 0;                              // is this really neccessary?
    llvm::Value* value = nullptr;           
    llvm::Function* function = nullptr;
    llvm::Type* llvm_type = nullptr;
};

struct symbol_table {        
    std::vector<signature_entry> master = {};
    std::vector<stack_frame> frames = {};
    
    program_data& data;
    
    void update(llvm::ValueSymbolTable& llvm);
    void push_new_frame();        
    void pop_last_frame();    
    std::vector<nat>& top();    
    expression& get(nat index);    
    void define(const expression& signature, const expression_list& definition, nat back_from, nat parent = 0);    
    void sort_top_by_largest();    
    symbol_table(program_data& data, const std::vector<expression>& builtins);         
    std::vector<std::string> llvm_key_symbols_in_table(llvm::ValueSymbolTable llvm);
    void debug();
    
};

std::string expression_to_string(const expression& given, symbol_table& stack);

std::vector<std::string> string_top(symbol_table& stack);
std::vector<expression> top(symbol_table& stack);

void print_stack(symbol_table& stack);
void print_index_top_stack(symbol_table &stack);
void print_llvm_symtable(llvm::ValueSymbolTable& table);

std::vector<expression> get_master(symbol_table& stack);
void print_master(symbol_table& stack);
void print_simply_master(symbol_table& stack);
#endif /* symbol_table_hpp */
