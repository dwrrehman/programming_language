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
    auto res = resolve_expression_list(unit, intrin::unit, main_function, state, flags.generate_code().at_top_level());
    error |= res.error;
    if (debug) debug_table(module, stack);

    clean(module);
    if (llvm::verifyFunction(*main_function)) append_return_0_statement(builder, context);
    if (llvm::verifyModule(*module, &llvm::errs())) error = true;    
    if (debug) {
        std::cout << "------------------ analysis ------------------- \n\n";
        print_resolved_unit(res, state);
        std::cout << "emitting the following LLVM: \n";
        std::cout << emit(module);
    }
    if (error) { throw "analysis error"; }
    else { return module; }
}
