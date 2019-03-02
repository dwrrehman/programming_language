//
//  main.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//
#include "nodes.hpp"


#include "compiler.hpp"
#include "interpreter.hpp"
#include "arguments.hpp"

//#include "llvm/IR/LLVMContext.h"
//#include "llvm/IR/Module.h"

#include <iostream>
#include <fstream>
#include <vector>

//llvm::LLVMContext context;
//llvm::Module module;

int main(int argc, const char** argv) {
    
    struct arguments args = get_commandline_arguments(argc, argv);
    
    if (args.error) {
        debug_arguments(args);
        return 1;
    }
    
    if (args.use_interpreter) {
        interpreter(args.files[0].data);
        return 0;
    }

    std::vector<struct action_tree> trees;
    trees.reserve(args.files.size());
    
    // set the llvm data.
        
    for (size_t i = 0; i < args.files.size(); i++) {
        trees[i] = frontend(args.files[i]);
        code_generation(trees[i]);
    }

    return 0;
}
