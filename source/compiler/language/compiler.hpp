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

std::unique_ptr<llvm::Module> frontend(struct file file, llvm::LLVMContext &context);
void optimize(llvm::Module* module);
std::string generate(llvm::Module* module, const struct file& file);
void delete_files(std::vector<std::string> object_files);
void link(std::vector<std::string> object_files, const struct arguments& arguments);


#endif /* compiler_hpp */
