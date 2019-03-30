//
//  compiler.hpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef compiler_hpp
#define compiler_hpp

#include "arguments.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include <string>
#include <vector>

std::unique_ptr<llvm::Module> frontend(struct file file, llvm::LLVMContext &context, bool is_metaprogram);

void optimize(llvm::Module& module);

void link(llvm::Module &program, std::unique_ptr<llvm::Module> &module);

llvm::Module& pop(std::vector<std::unique_ptr<llvm::Module>> &modules);


#endif /* compiler_hpp */
