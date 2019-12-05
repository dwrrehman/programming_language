//
//  symbol_table.cpp
//  language
//
//  Created by Daniel Rehman on 1908191.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "symbol_table.hpp"

#include "error.hpp"

#include "llvm/IR/ValueSymbolTable.h"

#include <iostream>
#include <iomanip>



void symbol_table::update(llvm::ValueSymbolTable& llvm) {        
//        for (auto& frame : frames) {
//            for (auto i = llvm.begin(); i != llvm.end(); i++) { 
//                auto key = i->getKey();                                 
//                auto value = i->getValue(); 
//                auto llsymbol = convert_raw_llvm_symbol_to_expression(key, value, *this, data, flags);                
//                bool found = false;
//                for (auto i : frame.indicies) 
//                    if (expressions_match(master[i].signature, llsymbol)) found = true;
//                if (not found) {
//                    master.push_back({llsymbol, {}}); // dont we need to push more than this?
//                    frame.indicies.push_back(master.size() - 1);
//                }
//            }
//        }
        print_warning_message(data.file.name, "unimplemented function called", 0,0);
    }
    
    void symbol_table::push_new_frame() { frames.push_back({frames.back().indicies}); }
        
    void symbol_table::pop_last_frame() { frames.pop_back(); }
    
    std::vector<nat>& symbol_table::top() { return frames.back().indicies; }
    
    expression& symbol_table::get(nat index) { return master[index].signature; }
    
    void symbol_table::define(const expression& signature, const expression_list& definition, nat back_from, nat parent) {
        // unfinsihed
        print_warning_message(data.file.name, "unimplemented function called", 0,0);
        // this function should do a check for if the signature is already 
        // defined in the current scope. if so, then simply overrite its data.
                
        frames[frames.size() - (++back_from)].indicies.push_back(master.size()); 
        master.push_back({signature, definition, parent});
        
        //we need to define it the LLVM symbol table!        
        // and we need to define it of the right type, as well.
        sort_top_by_largest();
    }

    void symbol_table::sort_top_by_largest() {
        std::stable_sort(top().begin(), top().end(), [&](nat a, nat b) {
            return get(a).symbols.size() > get(b).symbols.size(); 
        });
    }
    
    symbol_table::symbol_table(program_data& data, const std::vector<expression>& builtins) 
    : data(data) {
        
        master.push_back({});               // the null entry. a type (index) of 0 means it has no type.                
        for (auto signature : builtins) 
            master.push_back({signature, {}, {}});
        
        std::vector<nat> compiler_intrinsics = {};
        for (auto i = 0; i < builtins.size(); i++) compiler_intrinsics.push_back(i + 1);
        frames.push_back({compiler_intrinsics});
        sort_top_by_largest();
    }
    
    std::vector<std::string> symbol_table::llvm_key_symbols_in_table(llvm::ValueSymbolTable llvm) {
        std::vector<std::string> result = {};
        for (auto i = llvm.begin(); i != llvm.end(); i++)          
            result.push_back(i->getKey());
        return result;
    }    
    
    void symbol_table::debug() {
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
            std::cout << "\t" << std::setw(6) << j << ": ";
            std::cout << expression_to_string(entry.signature, *this) << "\n";
            
            if (entry.value) {
                std::cout << "\tLLVM value: \n";
                entry.value->print(llvm::errs());
            }
            if (entry.function) {
                std::cout << "\tLLVM function: \n";
                entry.function->print(llvm::errs());
            }
            
            j++;
        }
        std::cout << "}\n";
    }
    






















//std::vector<std::string> string_top(symbol_table& stack) {        
//    std::vector<std::string> result = {};
//    auto indicies = stack.top();
//    for (auto i : indicies) {
//        result.push_back(expression_to_string(stack.get(i), stack));
//    } 
//    return result;
//}
//
//void print_simply_master(symbol_table& stack) {
//    auto j = 0;
//    auto indicies = stack.top();
//    for (auto i : indicies) {
//        std::cout << j << ": " << expression_to_string(stack.master[i].signature, stack) << "\n";
//        j++;
//    }     
//}
//
//std::vector<expression> get_master(symbol_table& stack) {        
//    std::vector<expression> result = {};
//    for (auto entry : stack.master) result.push_back(entry.signature);
//    return result;
//}
//
//void print_index_top_stack(symbol_table &stack) {
//    expression_list e {top(stack)};
//    nat i = 0;
//    std::cout << "{\n";
//    for (auto f : e.list) {
//        std::cout << "\t" << i++ << ": " << expression_to_string(f, stack) << "\n";
//    }
//    std::cout << "}\n";
//}
//
//std::vector<expression> top(symbol_table& stack) {        
//    std::vector<expression> result = {};
//    auto indicies = stack.top();
//    for (auto i : indicies) {
//        result.push_back(stack.get(i));
//    }
//    return result;
//}
//
//void print_llvm_symtable(llvm::ValueSymbolTable& table) {
//    std::cout << "----------------printing LLVM symbol table: -----------------\n";
//    auto j = 0;
//    for (auto i = table.begin(); i != table.end(); i++) {
//        std::cout << j << ": ";
//        std::cout << "\tkey: \"" << i->getKey().str() << "\"\n";
//        
//        std::cout << "\tvalue: [[[";        
//        i->getValue()->print(llvm::outs()); 
//        std::cout << "]]]\n\n"; 
//        j++;        
//    }
//    std::cout << "---------------------------------------------\n";
//}
//
//void print_stack(symbol_table& stack) {
//    std::cout << "\n\n---------------- printing n3zqx2l stack -------------------\n\n";
//    for (int i = 0; i < stack.frames.size(); i++) {
//        std::cout << "----- FRAME # " << i << " -------------------\n";
//        for (int j = 0; j < stack.frames[i].indicies.size(); j++) {
//            
//            std::cout << "#" << j << " : " << expression_to_string(stack.master[stack.frames[i].indicies[j]].signature, stack);
//            auto parent = stack.master[stack.frames[i].indicies[j]].parent;
//            std::cout << "   ---> " << parent << "\n";
//            
//        }
//        std::cout << "---------------------------------------------\n\n";
//    }
//    std::cout << "\n\n-----------------------------------------------------------\n\n";
//}
//
//void print_master(symbol_table& stack) {
//    std::cout << "\n\n---------------- printing n3zqx2l master -------------------\n\n";
//    for (int i = 0; i < stack.master.size(); i++) {        
//        std::cout << "#" << i << " : " << expression_to_string(stack.master[i].signature, stack); 
//        auto parent = stack.master[i].parent;
//        std::cout << "   ---> " << parent << "\n";                        
//    }
//    std::cout << "\n\n-----------------------------------------------------------\n\n";
//}




