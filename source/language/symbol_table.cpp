//
//  symbol_table.cpp
//  language
//
//  Created by Daniel Rehman on 1908191.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "symbol_table.hpp"

#include "debug.hpp"
#include "builtins.hpp"

std::vector<std::string> string_top(symbol_table& stack) {        
    std::vector<std::string> result = {};
    auto indicies = stack.top();
    for (auto i : indicies) {
        result.push_back(expression_to_string(stack.get(i), stack));
    } 
    return result;
}

void print_simply_master(symbol_table& stack) {
    auto j = 0;
    auto indicies = stack.top();
    for (auto i : indicies) {
        std::cout << j << ": " << expression_to_string(stack.master[i].signature, stack) << "\n";
        j++;
    }     
}

std::vector<expression> get_master(symbol_table& stack) {        
    std::vector<expression> result = {};
    for (auto entry : stack.master) result.push_back(entry.signature);
    return result;
}

void print_index_top_stack(symbol_table &stack) {
    expression_list e {top(stack)};
    nat i = 0;
    std::cout << "{\n";
    for (auto f : e.list) {
        std::cout << "\t" << i++ << ": " << expression_to_string(f, stack) << "\n";
    }
    std::cout << "}\n";
}

std::vector<expression> top(symbol_table& stack) {        
    std::vector<expression> result = {};
    auto indicies = stack.top();
    for (auto i : indicies) {
        result.push_back(stack.get(i));
    }
    return result;
}

void print_llvm_symtable(llvm::ValueSymbolTable& table) {
    std::cout << "----------------printing LLVM symbol table: -----------------\n";
    auto j = 0;
    for (auto i = table.begin(); i != table.end(); i++) {
        std::cout << j << ": ";
        std::cout << "\tkey: \"" << i->getKey().str() << "\"\n";
        
        std::cout << "\tvalue: [[[";        
        i->getValue()->print(llvm::outs()); 
        std::cout << "]]]\n\n"; 
        j++;        
    }
    std::cout << "---------------------------------------------\n";
}

void print_stack(symbol_table& stack) {
    std::cout << "\n\n---------------- printing n3zqx2l stack -------------------\n\n";
    for (int i = 0; i < stack.frames.size(); i++) {
        std::cout << "----- FRAME # " << i << " -------------------\n";
        for (int j = 0; j < stack.frames[i].indicies.size(); j++) {
            
            std::cout << "#" << j << " : " << expression_to_string(stack.master[stack.frames[i].indicies[j]].signature, stack);
            auto parent = stack.master[stack.frames[i].indicies[j]].parent;
            std::cout << "   ---> " << parent << "\n";
            
        }
        std::cout << "---------------------------------------------\n\n";
    }
    std::cout << "\n\n-----------------------------------------------------------\n\n";
}

void print_master(symbol_table& stack) {
    std::cout << "\n\n---------------- printing n3zqx2l master -------------------\n\n";
    for (int i = 0; i < stack.master.size(); i++) {        
        std::cout << "#" << i << " : " << expression_to_string(stack.master[i].signature, stack); 
        auto parent = stack.master[i].parent;
        std::cout << "   ---> " << parent << "\n";                        
    }
    std::cout << "\n\n-----------------------------------------------------------\n\n";
}

