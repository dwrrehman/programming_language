#ifndef llvm_parser_hpp
#define llvm_parser_hpp

#include "analysis_ds.hpp"
#include "arguments.hpp"
#include "lists.hpp"

#include "llvm/Support/SourceMgr.h" 
#include "llvm/IR/Function.h"

bool parse_llvm_string_as_instruction(const std::string& given, llvm::Function*& original, state& state, llvm::SMDiagnostic& errors);
resolved_expression parse_llvm_string(llvm::Function*& function, const std::string& llvm_string, nat& pointer, state& state);

#endif /* llvm_parser_hpp */
