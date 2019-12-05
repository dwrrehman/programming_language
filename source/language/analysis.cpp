#include "analysis.hpp"

#include "debug.hpp"
#include "error.hpp"
#include "builtins.hpp"
#include "symbol_table.hpp"
#include "helpers.hpp"

#include "llvm/IR/Verifier.h"

static void verify(const file& file, llvm_module& module, resolved_expression_list& resolved_program) {
    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) {
        print_error_message(file.name, errors, 0, 0);
        resolved_program.error = true;
    }
}

std::string emit(const llvm_module& module) {
    std::string string = "";
    module->print(llvm::raw_string_ostream(string) << "", NULL);
    return string;
}

static void debug_program(llvm_module& module, const resolved_expression_list& resolved_program, state& state) {
    if (debug) {
        state.stack.debug();
        print_resolved_unit(resolved_program, state);
        llvm::outs() << emit(module);
    }
}

llvm_module analyze(expression_list program, const file& file, llvm::LLVMContext& context) {
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    llvm::IRBuilder<> builder(context);
    program_data data {file, module.get(), builder};
    symbol_table stack {data, builtins};
    state state {stack, data};
    
    auto main = create_main(builder, context, module);
    stack.sort_top_by_largest();
    
    prune_extraneous_subexpressions(program);
    auto resolved = resolve_expression_list(program, intrin::unit, main, state);
    remove_extraneous_insertion_points_in(module);
    move_lone_terminators_into_previous_blocks(module);
    delete_empty_blocks(module);
    
    if (not contains_final_terminator(main)) append_return_0_statement(builder, main, context);
    verify(file, module, resolved);    
    debug_program(module, resolved, state);
    
    if (resolved.error) exit(10);
    else return module;
}
