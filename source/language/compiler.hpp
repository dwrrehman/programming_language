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

void initialize_llvm();

std::vector<std::unique_ptr<llvm::Module>> frontend(const struct arguments &arguments, llvm::LLVMContext& context, bool error);

std::unique_ptr<llvm::Module> process(struct file file, llvm::LLVMContext &context);

void optimize(std::vector<std::unique_ptr<llvm::Module>>& modules);

std::string generate_object_file(std::unique_ptr<llvm::Module>& module, const struct file& file);

std::vector<std::string> generate_object_files(const struct arguments& arguments, bool error, std::vector<std::unique_ptr<llvm::Module>>& modules) ;

void delete_files(std::vector<std::string> object_files);

void link_and_emit_executable(std::vector<std::string> object_files, const struct arguments& arguments);

int interpret(std::string executable_name, std::vector<std::unique_ptr<llvm::Module>> &modules) ;

void set_data_layout(std::unique_ptr<llvm::Module>& main_module);



#endif /* compiler_hpp */
