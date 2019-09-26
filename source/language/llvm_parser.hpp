//
//  llvm_parser.hpp
//  language
//
//  Created by Daniel Rehman on 1909146.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef llvm_parser_hpp
#define llvm_parser_hpp

#include "helpers.hpp"

#include "analysis_ds.hpp"
#include "compiler.hpp"
#include "parser.hpp"
#include "builtins.hpp"
#include "symbol_table.hpp"
#include "lists.hpp"
#include "error.hpp"


#include "llvm/IR/LLVMContext.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/daniels_interpreter/MCJIT.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/GenericValue.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

bool parse_llvm_string_as_instruction(std::string given, llvm::Function*& original, state& state, llvm::SMDiagnostic& errors);

bool parse_llvm_string_as_function(std::string given, state& state, llvm::SMDiagnostic& errors);

llvm::Type* parse_llvm_string_as_type(std::string given, state& state, llvm::SMDiagnostic& errors);

void print_llvm_error(const llvm::SMDiagnostic &errors, state &state);

resolved_expression parse_llvm_string(llvm::Function*& function, std::string llvm_string, nat& pointer, state& state);

void interpret_file_as_llvm_string(const struct file &file, state &state); 
    
#endif /* llvm_parser_hpp */
