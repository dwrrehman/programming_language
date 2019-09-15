//
//  symbol_table.hpp
//  language
//
//  Created by Daniel Rehman on 1908191.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef symbol_table_hpp
#define symbol_table_hpp

#include "arguments.hpp"
#include "nodes.hpp"
#include "converters.hpp"
#include "debug.hpp"
#include "helpers.hpp"
#include "builtins.hpp"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include <iostream>


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

    flags flags;
    file_data& data;
    
    std::vector<signature_entry> master = {};
    std::vector<stack_frame> frames = {};
    
    void update(llvm::ValueSymbolTable& llvm) {        
        for (auto& frame : frames) {
            for (auto i = llvm.begin(); i != llvm.end(); i++) { 
                auto key = i->getKey();                                 
                auto value = i->getValue(); 
                auto llsymbol = convert_raw_llvm_symbol_to_expression(key, value, *this, data, flags);                
                bool found = false;
                for (auto i : frame.indicies) 
                    if (expressions_match(master[i].signature, llsymbol)) found = true;
                if (not found) {
                    master.push_back({llsymbol, {}}); // dont we need to push more than this?
                    frame.indicies.push_back(master.size() - 1);
                }
            }
        }
    }
    
    void push_new_frame() { frames.push_back({frames.back().indicies}); }
        
    void pop_last_frame() { frames.pop_back(); }
    
    std::vector<nat>& top() { return frames.back().indicies; }
    
    expression& get(nat index) { return master[index].signature; }
    
    void define(expression signature, expression_list definition, nat back_from, nat parent = 0) {
        // unfinsihed
        
        // this function should do a check for if the signature is aclready defined in the current scope. if so, then simply overrite its data.
                
        frames[frames.size() - (++back_from)].indicies.push_back(master.size()); 
        master.push_back({signature, definition, parent});
        
        //we need to define it the LLVM symbol table!        
        // and we need to define it of the right type, as well.
        sort_top_by_largest();
    }
    
    void expose(nat desired_signature, expression new_signature, 
                  nat source_abstraction, nat destination_frame) {
        // unimplemented
    }
    
    void sort_top_by_largest() {
        std::sort(top().begin(), top().end(), [&](nat a, nat b) {
            return get(a).symbols.size() > get(b).symbols.size(); 
        });
    }
    
    symbol_table(file_data& data, struct flags flags, std::vector<expression> builtins) 
    : data(data), flags(flags) {
        
        master.push_back({});               // the null entry. a type (index) of 0 means it has no type.                
        for (auto signature : builtins) 
            master.push_back({signature, {}, {}});
        
        std::vector<nat> compiler_intrinsics = {};
        for (auto i = 0; i < builtins.size(); i++) compiler_intrinsics.push_back(i + 1);
        frames.push_back({compiler_intrinsics});
        sort_top_by_largest();
    }
    
    std::vector<std::string> llvm_key_symbols_in_table(llvm::ValueSymbolTable llvm) {
        std::vector<std::string> result = {};
        for (auto i = llvm.begin(); i != llvm.end(); i++)          
            result.push_back(i->getKey());
        return result;
    }    
    
    void debug() {
        std::cout << "---- debugging stack: ----\n";
     
        std::cout << "printing frames: \n";        
        for (auto i = 0; i < frames.size(); i++) {
            std::cout << "\t ----- FRAME # "<<i<<"---- \n\t\tidxs: { ";
            for (auto index : frames[i].indicies) {
                std::cout << index << " "; 
            }
            std::cout << "}\n";
        }
        
        std::cout << "\nmaster: {\n";
        auto j = 0;
        for (auto entry : master) {            
            std::cout << "\t" << j << ": {\"";
            //print_expression_line(entry.signature); 
            std::cout << "\" : [ parent = " << entry.parent << "]\n";
            std::cout << "LLVM value: \n";
            if (entry.value) entry.value->print(llvm::errs());
            std::cout << "LLVM function: \n";
            if (entry.function) entry.function->print(llvm::errs());
            
            j++;
        }
        std::cout << "}\n";
    }
    
};

std::string expression_to_string(expression given, symbol_table& stack);

std::vector<std::string> string_top(symbol_table& stack);
std::vector<expression> top(symbol_table& stack);

void print_stack(symbol_table& stack);
void print_index_top_stack(symbol_table &stack);
void print_llvm_symtable(llvm::ValueSymbolTable& table);

std::vector<expression> get_master(symbol_table& stack);
void print_master(symbol_table& stack);
void print_simply_master(symbol_table& stack);
#endif /* symbol_table_hpp */
