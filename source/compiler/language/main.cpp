//
//  main.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//
#include "nodes.hpp"    // dummy, work in progress.




/*
 
  add user cli hooks to make the compiler stop at any stage, and output the internal represetnation to the user.
 
 
 like:
 
 
 --emit=preprocessed  or    -p
 --emit=ast           or    -ast
 --emit=actiontree   or     -at
 
 --emit-llvm       or       -llvm
 
 
 
 we need these, they are useful for debugging and they make the compiler alittle bit more featureful.
 
 
 
 
 
 */







#include "compiler.hpp"
#include "interpreter.hpp"
#include "arguments.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "llvm/IR/IRBuilder.h"

#include <iostream>
#include <fstream>
#include <vector>

llvm::LLVMContext context;

int main(int argc, const char** argv) {
      
    struct arguments args = get_commandline_arguments(argc, argv);
    
    if (args.error) {
        debug_arguments(args);
        return 1;
        
    } else if (args.use_interpreter) {
        interpreter(args.files[0].data);
        return 0;
    }

    std::vector<llvm::Module*> modules;
    modules.reserve(args.files.size());
    
    for (size_t i = 0; i < args.files.size(); i++) {
        modules[i] = frontend(args.files[i]);
    }

    return 0;
}
