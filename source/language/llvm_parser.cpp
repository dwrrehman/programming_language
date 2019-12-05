#include "llvm_parser.hpp"
#include "builtins.hpp"
#include "error.hpp"

#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/AsmParser/Parser.h"

#include <sstream>
#include <iostream>

///important note about this function:
/// it leaves artifacts in the function after use, which must be removed: 
/// any occurence of a unreachable statement which is directly preceeded by 
/// a llvm.do_nothing() call, should be removed before execution of the function.

bool parse_llvm_string_as_instruction(const std::string& given, llvm::Function*& original, state& state, llvm::SMDiagnostic& errors) {
    static nat num = 0;
    std::string body = "";
    original->print(llvm::raw_string_ostream(body) << "");    
    body.pop_back(); // delete the newline
    body.pop_back(); // delete the close brace      
    body += given + "\n call void @llvm.donothing() \n unreachable \n } \n";     
    const std::string current_name = original->getName(); 
    original->setName("_anonymous_" + std::to_string(num++));
    llvm::MemoryBufferRef reference(body, "<llvm-string>");
    llvm::ModuleSummaryIndex my_index(true);
    if (llvm::parseAssemblyInto(reference, state.data.module, &my_index, errors)) {        
        original->setName(current_name);
        return false;
    } else {
        original->getBasicBlockList().clear();
        original = state.data.module->getFunction(current_name);
        return true;
    }
}

resolved_expression parse_llvm_string(llvm::Function*& function, const std::string& llvm_string, nat& pointer, state& state) {
    llvm::SMDiagnostic instruction_errors, function_errors, type_errors;
    llvm::ModuleSummaryIndex my_index(true);
    llvm::MemoryBufferRef reference(llvm_string, "<llvm-string>");
    
    if (not llvm::parseAssemblyInto(reference, state.data.module, &my_index, function_errors) or
        parse_llvm_string_as_instruction(llvm_string, function, state, instruction_errors)) {
        pointer++;
        return {intrin::unit_value, {}, false};
    } else if (auto llvm_type = llvm::parseType(llvm_string, type_errors, *state.data.module)) {
        pointer++;
        return {intrin::typeless, {}, false, llvm_type};        
    } else {
        print_llvm_error(function_errors, state);
        print_llvm_error(instruction_errors, state);        
        print_llvm_error(type_errors, state); 
        return {0, {}, true};
    }
}
