//
//  symbol_table.cpp
//  language
//
//  Created by Daniel Rehman on 1908191.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "symbol_table.hpp"




void print_stack() {
    std::cout << "\n\n---------------- printing MASTER stack -------------------\n\n";
    for (int i = 0; i < frames.size(); i++) {
        std::cout << "----- FRAME # " << i << " -------------------\n";
        for (int j = 0; j < frames[i].indicies.size(); j++) {
            
            std::cout << "#" << j << " : " << expression_to_string(master[frames[i].indicies[j]].signature) << "   :    [ "; 
            
            for (auto h : master[frames[i].indicies[j]].parents) {
                std::cout << h << " ";
            } std::cout << "]\n";
        }
        std::cout << "---------------------------------------------\n\n";
    }
    std::cout << "\n\n-----------------------------------------------------------\n\n";
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
