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



static void debug_table(const std::unique_ptr<llvm::Module, std::default_delete<llvm::Module> > &module, symbol_table &stack) {
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





std::unique_ptr<llvm::Module> analyze(translation_unit unit, llvm::LLVMContext& context, struct file file) {
    
    srand((unsigned)time(nullptr));
    llvm::IRBuilder<> builder(context);
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    auto triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(triple);    
    auto main_function = create_main(builder, context, module);    
    declare_donothing(builder, module);
    
    bool error = false;
    flags flags = {};
    translation_unit_data data = {file, module.get(), builder};
    symbol_table stack {data, flags, builtins};     
    state state = {stack, data, error};
    
    auto& body = unit.list.expressions;    
    std::vector<expression> parsed_body = {};
    for (auto expression : body) {        
                
        expression.type = intrin::unit;
        auto solution = res(expression, state, flags.generate_code().at_top_level());
        if (solution.erroneous) error = true;
        else parsed_body.push_back(solution);            
    }
    body = parsed_body;
    
    debug_table(module, stack);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    if (llvm::verifyFunction(*main_function)) append_return_0_statement(builder, context);
    if (llvm::verifyModule(*module, &llvm::errs())) error = true;
    if (debug) print_translation_unit(unit, file);    
    
    if (debug) {
        std::cout << "emitting the following LLVM: \n";
        module->print(llvm::errs(), NULL); // temp
    }
    
    if (error) { throw "analysis error"; }
    else { return module; }
}

//    if (contains_top_level_runtime_statement(body)) found_main = true; 
//    else if (found_main) main_function->eraseFromParent();

