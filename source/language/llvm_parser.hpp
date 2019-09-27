//
//  llvm_parser.hpp
//  language
//
//  Created by Daniel Rehman on 1909146.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef llvm_parser_hpp
#define llvm_parser_hpp

#include "analysis_ds.hpp"
#include "arguments.hpp"
#include "lists.hpp"

#include "llvm/Support/SourceMgr.h" 
#include "llvm/IR/Function.h"

#include <string>


bool parse_llvm_string_as_instruction(const std::string& given, llvm::Function*& original, state& state, llvm::SMDiagnostic& errors);

bool parse_llvm_string_as_function(const std::string& given, state& state, llvm::SMDiagnostic& errors);

llvm::Type* parse_llvm_string_as_type(const std::string& given, state& state, llvm::SMDiagnostic& errors);

void print_llvm_error(const llvm::SMDiagnostic& errors, state& state);

resolved_expression parse_llvm_string(llvm::Function*& function, const std::string& llvm_string, nat& pointer, state& state);

void interpret_file_as_llvm_string(const file &file, state &state); 
    
#endif /* llvm_parser_hpp */
