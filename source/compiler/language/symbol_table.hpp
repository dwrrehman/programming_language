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

struct signature_entry;
using master = std::vector<struct signature_entry>; 
using nat = size_t;

struct stack_frame {
    llvm::ValueSymbolTable& llvm;
    std::vector<nat> indicies = {};  
};

struct signature_entry {
    expression signature = {};
    expression_list definition = {};
    nat precedence = 0;
    nat parent = 0;
    llvm::Value* value = nullptr;
    llvm::Function* function = nullptr;
};

class symbol_table {
public:
    flags flags;
    file_data& data;
    master master = {};
    std::vector<struct stack_frame> frames = {};
              
    void push(llvm::ValueSymbolTable& llvm) {        
        frames.push_back({llvm, frames.back().indicies});
    }    
    void pop() {
        frames.pop_back();        
    }
    
    void update() {
        /// job:
        /*
         for each stackframe, 
         
            for each llsymbol in that sf's LLST, 
         
                if that llsymbol is not in our indexed stackframe, 
                    (which we will determine by indexing each stackframe index into the master, 
                    and checking if the symbol stringified versions match...                           TODO:  we need to code a fucntion which takes a typical llvm function signture, such as:       void @f(i32 %x, i32 %y)     and convert it into a n3zqx2l signtaure,     in this case:        (f ((x) `i32`)  ((y) `i32`)) () (_type)   
                    )
         
            then we will create a new entry in master, (push_back(something)),
         
            and then fill its data with what we know to be the key and value of the thing we found from searching the LLST.
                  
         */
        
        for (auto& frame : frames) {
            for (auto i = frame.llvm.begin(); i != frame.llvm.end(); i++) { 
                auto key = i->getKey();                                 
                auto value = i->getValue(); 
                auto llsymbol = convert_raw_llvm_symbol_to_expression(key, value, *this, data, flags);                
                bool found = false;
                for (auto i : frame.indicies) {
                    if (expressions_match(master[i].signature, llsymbol)) { 
                        found = true;
                    }
                }
                if (not found) {
                    master.push_back({llsymbol, {}}); /// we need to push its definition properly too!!! 
                    frame.indicies.push_back(master.size() - 1);
                }
            }
        }
    }
    
    std::vector<nat>& top() {
        update();
        return frames.back().indicies;        
    }
    
    expression& lookup(nat index) {
        update();
        return master[index].signature;        
    }    
    
    void define(expression signature, expression_list definition,
                nat stack_frame_index, nat parent = 0) {   
        update();
        frames[frames.size() - 1 - stack_frame_index].indicies.push_back(master.size()); 
        master.push_back({signature, definition, parent});
        //we need to define it the LLVM symbol table!        
        // and we need to define it of the right type, as well.
        
        // unfinsihed
    }
    
    void expose(nat desired_signature, expression new_signature, 
                  nat source_abstraction, nat destination_frame) {
        update();
        
        // unimplemented
    }
    
    symbol_table(file_data& data, struct flags flags, 
                 std::vector<expression> builtins) : data(data), flags(flags) {         
        master.push_back({}); // the null entry. a type (index) of 0 means it has no type.        
        
        for (auto signature : builtins) {
            master.push_back({signature, {}, {}});
        }
        
        std::vector<nat> compiler_intrinsics = {};
        for (auto i = 0; i < builtins.size(); i++) compiler_intrinsics.push_back(i + 1);
        frames.push_back({data.module->getValueSymbolTable(), compiler_intrinsics});
    }
    
    
    
    
    std::vector<std::string> llvm_key_symbols_in_table(llvm::ValueSymbolTable llvm) {
        std::vector<std::string> result = {};
        for (auto i = llvm.begin(); i != llvm.end(); i++) {            
            result.push_back(i->getKey());            
        }
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
            std::cout << "\t\tllvm keys: ";
            print(llvm_key_symbols_in_table(frames[i].llvm));
            std::cout << "\n";
        }
        
        std::cout << "\nmaster: {\n";
        auto j = 0;
        for (auto entry : master) {            
            std::cout << "\t" << j << ": {\"";
            print_expression_line(entry.signature); 
            std::cout << "\" : [ " << entry.parent << "] :: {\n";
            //print_abstraction_definition(entry.definition, 0);      /// TODO: fix me so we can print stuff.            
            std::cout << "}\n";
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
