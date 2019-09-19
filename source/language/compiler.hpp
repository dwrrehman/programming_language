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
#include <vector>


using llvm_modules = std::vector<std::unique_ptr<llvm::Module>>;
using llvm_module = std::unique_ptr<llvm::Module>;


void initialize_llvm();
llvm_module process(file file, llvm::LLVMContext &context);
llvm_modules frontend(arguments arguments, llvm::LLVMContext& context);
llvm_module link(llvm_modules modules);
void set_data_layout(llvm_module& module);
void interpret(llvm_module& module, arguments arguments);
void optimize(llvm_module& module);
std::string generate_object_file(llvm_module& module, arguments arguments);
void emit_executable(std::string object_file, arguments arguments);
#endif /* compiler_hpp */
