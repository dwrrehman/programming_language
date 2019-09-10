//
//  analysis.cpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "analysis.hpp"

#include "builtins.hpp"
#include "debug.hpp"
#include "helpers.hpp"
#include "lists.hpp"
#include "symbol_table.hpp"
#include "llvm/IR/Verifier.h"
#include "llvm/Target/TargetMachine.h"


static void debug_table(const std::unique_ptr<llvm::Module>& module, symbol_table& stack) {
    std::cout << "NOTE: updating stack...\n";
    stack.update();
    
    std::cout << "print_stack: \n"; 
    print_stack(stack);
    
    std::cout << "print_master: \n";
    print_master(stack);
    
    std::cout << "print_index_top_stack: \n";
    print_index_top_stack(stack);
    
    std::cout << "print_simply_master: \n";
    print_simply_master(stack);
    
    std::cout << "print_llvm_symtable: \n";
    print_llvm_symtable(module->getValueSymbolTable());
    
    std::cout << "\n\n\n\n";
    stack.debug();
    std::cout << "\n\n\n\n";
}

std::unique_ptr<llvm::Module> analyze(expression_list unit, file file, llvm::LLVMContext& context) {
    
    srand((unsigned)time(nullptr));
    llvm::IRBuilder<> builder(context);
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    auto triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(triple);
    auto main_function = create_main(builder, context, module);
    declare_donothing(builder, module);
    
    bool error = false;
    flags flags = {};
    file_data data = {file, module.get(), builder};
    symbol_table stack {data, flags, builtins};
    state state = {stack, data, error};
    
    //auto new_unit = resolve(unit, main_function, state, flags.generate_code().at_top_level());
    
    
    
    llvm::SMDiagnostic instruction_errors;
    auto s = unit.list.back().symbols.back().llvm.literal.value;
    bool success = parse_llvm_string_as_instruction(s, main_function, state, instruction_errors);
    
    if (success) {
        std::cout << "yay it worked.\n";
        std::cin.get();
        std::cout << "ins: llvm: "; // TODO: make this have color!
        instruction_errors.print(state.data.file.name.c_str(), llvm::errs()); // temp
    } else {
        std::cout << "darn it failed.\n";
        std::cin.get();
        std::cout << "ins: llvm: "; // TODO: make this have color!
        instruction_errors.print(state.data.file.name.c_str(), llvm::errs()); // temp
    }
        
    //if (debug) debug_table(module, stack);    

    if (llvm::verifyFunction(*main_function)) append_return_0_statement(builder, context);
    if (llvm::verifyModule(*module, &llvm::errs())) error = true;    
    if (debug) {         
        std::cout << "------------------ analysis ------------------- \n\n";
        print_translation_unit(unit, file); // these should be new unit
        print_expression_list_line(unit); // these should be new unit
        std::cout << "emitting the following LLVM: \n";
        module->print(llvm::errs(), NULL); // temp
    }    
    if (error) { throw "analysis error"; }
    else { return module; }
}

//    if (contains_top_level_runtime_statement(body)) found_main = true; 
//    else if (found_main) main_function->eraseFromParent();

