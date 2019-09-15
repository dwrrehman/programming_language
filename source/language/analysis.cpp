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
#include "llvm_parser.hpp"
#include "symbol_table.hpp"

#include "llvm/IR/Verifier.h"
#include "llvm/Target/TargetMachine.h"

static void unitize_all_in(expression_list& unit) {
    for (auto& e : unit.list) e.type = intrin::unit; 
}

std::unique_ptr<llvm::Module> analyze(expression_list unit, file file, llvm::LLVMContext& context) {
    
    srand((unsigned)time(nullptr));
    llvm::IRBuilder<> builder(context);
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    auto triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(triple);
    auto main_function = create_main(builder, context, module);
    call_donothing(builder, module);
    
    bool error = false;
    flags flags {};
    file_data data {file, module.get(), builder};
    symbol_table stack {data, flags, builtins};
    state state {stack, data, error};
    
    // temp;
    //interpret_file_as_llvm_string(file, state);
    //builder.CreateCall(module->getFunction("start")); 
    
    unitize_all_in(unit);
    resolved_expression_list new_unit = resolve(unit, main_function, state, flags.generate_code().at_top_level());

    
    
    if (llvm::verifyFunction(*main_function)) append_return_0_statement(builder, context);
    if (llvm::verifyModule(*module, &llvm::errs())) error = true;
    
    if (debug) {
        std::cout << "------------------ analysis ------------------- \n\n";
        
        std::cout << "emitting the following LLVM: \n";
        module->print(llvm::errs(), NULL); // temp
        
    }
    if (error) { throw "analysis error"; }
    else { return module; }
}

//    if (contains_top_level_runtime_statement(body)) found_main = true; 
//    else if (found_main) main_function->eraseFromParent();

