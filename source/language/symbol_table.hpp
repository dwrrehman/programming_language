#ifndef symbol_table_hpp
#define symbol_table_hpp

#include "lists.hpp"
#include "nodes.hpp"
#include "analysis_ds.hpp"

#include "llvm/IR/Function.h"

#include <vector>

struct stack_frame {
    std::vector<nat> indicies = {};  
};

struct signature_entry {
    expression signature = {};
    expression_list definition = {}; 
    nat precedence = 0;
    llvm::Value* value = nullptr;           
    llvm::Function* function = nullptr;
    llvm::Type* llvm_type = nullptr;
};

struct symbol_table {        
    std::vector<signature_entry> master = {};
    std::vector<stack_frame> frames = {};
    struct program_data& data;
    
    symbol_table(program_data& data, const std::vector<expression>& builtins);
    void update(llvm::ValueSymbolTable& llvm);
    void push_new_frame();        
    void pop_last_frame();
    std::vector<nat>& top();    
    expression& get(nat index);    
    void define(const expression& signature, const expression_list& definition, nat back_from, nat parent = 0);    
    void sort_top_by_largest();
    std::vector<std::string> llvm_key_symbols_in_table(llvm::ValueSymbolTable llvm);
};

std::string expression_to_string(const expression& given, symbol_table& stack);

#endif /* symbol_table_hpp */
