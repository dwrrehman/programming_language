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


static void verify(file file, const std::unique_ptr<llvm::Module>& module, resolved_expression_list& resolved_program) {
    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) {
        print_error_message(file.name, errors, 0, 0);
        resolved_program.error = true;
    }
}

static void debug_program(std::unique_ptr<llvm::Module> & module, resolved_expression_list resolved_program, state& state) {
    if (debug) {        
        std::cout << "------------------ stack state ------------------- \n\n";
        state.stack.debug();
        
        std::cout << "------------------ resolution ------------------- \n\n";
        print_resolved_unit(resolved_program, state);
        
        std::cout << "------------ emitting LLVM: ----------- \n";
        std::cout << emit(module);
    }
}

std::unique_ptr<llvm::Module> analyze(expression_list program, file file, llvm::LLVMContext& context) {
    srand((unsigned)time(nullptr));
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    auto triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(triple);
    llvm::IRBuilder<> builder(context);
    flags flags {};
    program_data data {file, module.get(), builder};
    symbol_table stack {data, {}, builtins};
    state state {stack, data};
    auto main = create_main(builder, context, module);
    call_donothing(builder, module);
    stack.sort_top_by_largest();
    prune_extraneous_subexpressions_in_expression_list(program);
    auto resolved_program = resolve_expression_list(program, intrin::unit, main, state, flags.generate_code().at_top_level());
    remove_extraneous_insertion_points_in(module);
    move_lone_terminators_into_previous_blocks(module);
    delete_empty_blocks(module);
    append_return_0_statement(builder, main, context);
    verify(file, module, resolved_program);    
    debug_program(module, resolved_program, state); 
    if (resolved_program.error) throw "analysis error";
    else return module;
}
