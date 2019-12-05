#ifndef compiler_hpp
#define compiler_hpp

#include "arguments.hpp"
#include "analysis_ds.hpp"

void initialize_llvm();
llvm_modules frontend(const arguments& arguments, llvm::LLVMContext& context); 
llvm_module link(llvm_modules&& modules);
void set_data_layout(llvm_module& module);
void interpret(llvm_module& module, const arguments& arguments);
void optimize(llvm_module& module);
std::string generate_object_file(llvm_module& module, const arguments& arguments);
void emit_executable(const std::string& object_file, const arguments& arguments);

#endif /* compiler_hpp */
