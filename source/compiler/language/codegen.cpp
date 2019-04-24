//
//  codegen.cpp
//  language
//
//  Created by Daniel Rehman on 1903085.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "codegen.hpp"
#include "compiler.hpp"
#include "analysis.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

//#include "llvm/ADT/APFloat.h"
//#include "llvm/ADT/STLExtras.h"
//#include "llvm/IR/BasicBlock.h"
//#include "llvm/IR/Constants.h"
//#include "llvm/IR/DerivedTypes.h"
//#include "llvm/IR/Function.h"

//#include "llvm/IR/Type.h"
//#include "llvm/IR/Verifier.h"
//
//#include "llvm/AsmParser/Parser.h"


static std::string module_name(const std::string filename) {
    static size_t module_number = 0;
    return filename + std::to_string(module_number++);
}

std::unique_ptr<llvm::Module> generate(const translation_unit unit, struct file file, llvm::LLVMContext &context) {
    llvm::IRBuilder<> builder(context);
    auto module = llvm::make_unique<llvm::Module>(module_name(file.name), context);
    return module;
}
