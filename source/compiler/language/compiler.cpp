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

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Linker/Linker.h"

#include <iostream>
#include <fstream>
#include <vector>

std::unique_ptr<llvm::Module> frontend(struct file file, llvm::LLVMContext &context) {
    return generate(analyze(parse(preprocess(file.data, file.name), file.name), file.name), file.name, context);
}

void optimize(llvm::Module& module) {
    // call llvm something.
}

void link(llvm::Module &program, std::unique_ptr<llvm::Module> &module) {
    if (llvm::Linker::linkModules(program, std::move(module))) {
        std::cout << "Linking Error\n";
        exit(1);
    }
}

llvm::Module& pop(std::vector<std::unique_ptr<llvm::Module>> &modules) {
    auto& program = *modules.back();
    modules.pop_back();
    return program;
}
