#include "analysis.hpp"

#include "debug.hpp"
#include "error.hpp"
#include "builtins.hpp"
#include "symbol_table.hpp"
#include "resolver.hpp"

#include "llvm/IR/Verifier.h"



static inline void append_return_0_statement(llvm::IRBuilder<> &builder, llvm::Function* main_function, llvm::LLVMContext& context) {
    llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    builder.SetInsertPoint(&main_function->getBasicBlockList().back());
    builder.CreateRet(value);
}

static inline llvm::Function* create_main(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm_module& module) {
    std::vector<llvm::Type*> state = {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()};
    auto main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), state, false);
    auto main_function = llvm::Function::Create(main_type, llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main_function));
    return main_function;
}

void prune_extraneous_subexpressions(expression_list& given);
static inline void prune_extraneous_subexpressions_in_expression(expression& given) {
    while (given.symbols.size() == 1
           and subexpression(given.symbols[0])
           and given.symbols[0].expressions.list.size() == 1
           and given.symbols[0].expressions.list.back().symbols.size()) {
        auto save = given.symbols[0].expressions.list.back().symbols;
        given.symbols = save;
    }
    for (auto& symbol : given.symbols) if (subexpression(symbol)) prune_extraneous_subexpressions(symbol.expressions);
}

void prune_extraneous_subexpressions(expression_list& given) {
    for (auto& expression : given.list) prune_extraneous_subexpressions_in_expression(expression);
}

static inline void verify(const file& file, llvm_module& module, resolved_expression_list& resolved_program) {
    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) {
        print_error_message(file.name, errors, 0, 0);
        resolved_program.error = true;
    }
}

static inline void debug_program(llvm_module& module, const resolved_expression_list& resolved_program, state& state) {
    if (debug) {
        debug_table(state.stack);
        print_resolved_unit(resolved_program, state);
        module->print(llvm::outs(), nullptr);
    }
}

llvm_module analyze(expression_list program, const file& file, llvm::LLVMContext& context) {
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    llvm::IRBuilder<> builder(context);
    program_data data {file, module.get(), builder};
    symbol_table stack {data, builtins};
    state state {stack, data};
    stack.sort_top_by_largest();
    auto main = create_main(builder, context, module);
    builder.CreateCall(llvm::Intrinsic::getDeclaration(module.get(), llvm::Intrinsic::donothing));
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
