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

#include "llvm/Transforms/Utils/BasicBlockUtils.h"

void print_resolved_list(resolved_expression_list list, nat depth, state& state);
void print_resolved_expr(resolved_expression expr, nat depth, state& state);

void print_resolved_expr(resolved_expression expr, nat depth, state& state) {
    prep(depth); std::cout << "[error = " << std::boolalpha << expr.error << "]\n";
    prep(depth); std::cout << "index = " << expr.index << " :: " 
    << expression_to_string(state.stack.get(expr.index), state.stack) << "\n";
    
    if (expr.llvm_type) { prep(depth); std::cout << "llvm type = "; expr.llvm_type->print(llvm::errs()); }
    std::cout << "\n";
    nat i = 0;
    for (auto arg : expr.args) {
        prep(depth + 1); std::cout << "argument #" << i++ << ": \n";
        print_resolved_list(arg, depth + 2, state);
        prep(depth); std::cout << "\n";
    }
} 

void print_resolved_list(resolved_expression_list list, nat depth, state& state) {
    prep(depth); std::cout << "[error = " << std::boolalpha << list.error << "]\n";
    nat i = 0;
    for (auto expr : list.list) {        
        prep(depth + 1); std::cout << "expression #" << i++ << ": \n"; 
        print_resolved_expr(expr, depth + 2, state);
        prep(depth); std::cout << "\n";
    }
}

void print_resolved_unit(resolved_expression_list unit, state& state) {
    std::cout << "---------- printing resolved tranlation unit: ------------\n\n";
    print_resolved_list(unit, 0, state);
}


bool is_donothing_call(llvm::Instruction* ins) {
    if (not ins) return false;
    return std::string(ins->getOpcodeName()) == "call" and std::string(ins->getOperand(0)->getName()) == "llvm.donothing";
}

static void delete_empty_blocks(std::unique_ptr<llvm::Module> &module) {
    for (auto& function : module->getFunctionList()) {        
        llvm::SmallVector<llvm::BasicBlock*, 100> blocks = {};        
        for (auto& block : function.getBasicBlockList()) {
            if (block.empty()) blocks.push_back(&block);
        }
        llvm::DeleteDeadBlocks(blocks);
    }
}

void clean(std::unique_ptr<llvm::Module>& module) {    
    for (auto& function : module->getFunctionList()) {
        for (auto& block : function.getBasicBlockList()) {            
            auto ins = block.getTerminator();
            if (ins and std::string(ins->getOpcodeName()) == "unreachable" 
                and is_donothing_call(ins->getPrevNonDebugInstruction())) {
                ins->getPrevNonDebugInstruction()->eraseFromParent();                
                ins->eraseFromParent();
            }
        }
    }
    delete_empty_blocks(module);    
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
 
    typify(unit, intrin::unit);
    auto res = resolve_expression_list(unit, main_function, state, flags.generate_code().at_top_level());
                
    clean(module);
    if (llvm::verifyFunction(*main_function)) append_return_0_statement(builder, context);
    if (llvm::verifyModule(*module, &llvm::errs())) error = true;    
    if (debug) {        
        std::cout << "------------------ analysis ------------------- \n\n";
        print_resolved_unit(res, state);        
        std::cout << "emitting the following LLVM: \n";
        module->print(llvm::errs(), NULL);
    }
    if (error) { throw "analysis error"; }
    else { return module; }
}
