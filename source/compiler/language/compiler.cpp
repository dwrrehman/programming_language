//
//  compiler.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "compiler.hpp"
#include "arguments.hpp"
#include "preprocessor.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "nodes.hpp"
#include "analysis.hpp"
#include "codegen.hpp"
#include "corrector.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Linker/Linker.h"

#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/AggressiveInstCombine/AggressiveInstCombine.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <iostream>
#include <fstream>
#include <vector>

std::unique_ptr<llvm::Module> frontend(struct file file, llvm::LLVMContext &context) {
    return generate(analyze(correct(parse(file, context), file), file), file, context);
}


void optimize(llvm::Module& module) {
    // use a pass manager, and string together as many passes as possible.
}

void link(llvm::Module &program, std::unique_ptr<llvm::Module> &module) {

    // however, what we are probably going to do, is either call the api for LLD, or simply make a system call to lld.
    /// this method of linking might be useful when making the interpreter:

    //    if (llvm::Linker::linkModules(program, std::move(module))) {  // shouldnt we call the systme linker, "lld", and get good LTOs??
    //        std::cout << "Linking Error\n"; //TODO: print linking errors, by making a system call to "lld"
    //        exit(1);
    //    }

}

llvm::Module& pop(std::vector<std::unique_ptr<llvm::Module>> &modules) {
    auto& program = *modules.back();
    modules.pop_back();
    return program;
}
