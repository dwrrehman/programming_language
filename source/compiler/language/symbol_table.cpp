//
//  symbol_table.cpp
//  language
//
//  Created by Daniel Rehman on 1908191.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "symbol_table.hpp"

#include "debug.hpp"







/// TODO: put this in analysis or some other important file. this is a avery improtant function.

std::string expression_to_string(expression given, symbol_table& stack) {
    std::string result = "(";
    size_t i = 0;
    for (auto symbol : given.symbols) {
        if (symbol.type == symbol_type::identifier) result += symbol.identifier.name.value;
        else if (symbol.type == symbol_type::subexpression) {
            result += "(" + expression_to_string(symbol.subexpression, stack) + ")";
        }
        if (i < given.symbols.size() - 1) result += " ";
        i++;
    }
    result += ")";
    if (given.llvm_type) {
        std::string type = "";
        given.llvm_type->print(llvm::raw_string_ostream(type) << "");
        result += " " + type;
    } else if (given.type) {
        result += " " + expression_to_string(stack.lookup(given.type), stack);
    }
    return result;
}



static void print_symtable(llvm::ValueSymbolTable& table) {
    std::cout << "----------------printing symbol table: -----------------\n";
    for (auto i = table.begin(); i != table.end(); i++) {
        std::cout << "key: \"" << i->getKey().str() << "\"\n";
        std::cout << "value: [[[\n\n";
        i->getValue()->print(llvm::outs()); 
        std::cout << "]]]\n\n"; 
        
        std::cout << "here is the result of instead using .first and .second: \n";
        std::cout << "first: \"" << i->first().str() << "\"\n";
        std::cout << "second: [[[\n\n";
        i->second->print(llvm::outs());
        std::cout << "]]] \n\n";
    }
    std::cout << "---------------------------------------------\n";
}

void print_stack(symbol_table& stack) {
    std::cout << "\n\n---------------- printing MASTER stack -------------------\n\n";
    for (int i = 0; i < stack.frames.size(); i++) {
        std::cout << "----- FRAME # " << i << " -------------------\n";
        for (int j = 0; j < stack.frames[i].indicies.size(); j++) {
            
            std::cout << "#" << j << " : " << expression_to_string(stack.master[stack.frames[i].indicies[j]].signature, stack) << "   :    [ "; 
            
            for (auto h : stack.master[stack.frames[i].indicies[j]].parents) {
                std::cout << h << " ";
            } std::cout << "]\n";
        }
        std::cout << "---------------------------------------------\n\n";
    }
    std::cout << "\n\n-----------------------------------------------------------\n\n";
}
