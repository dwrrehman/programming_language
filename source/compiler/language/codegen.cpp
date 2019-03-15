//
//  codegen.cpp
//  language
//
//  Created by Daniel Rehman on 1903085.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "codegen.hpp"

#include "analysis.hpp"


#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/IRBuilder.h"


llvm::Module* code_generation(struct action_tree tree, llvm::LLVMContext context) {
    llvm::IRBuilder<> Builder(context);
    
    return nullptr;
}



int main() {
    
    //llvm::parseAssemblyInto(MemoryBufferRef F, Module *M, ModuleSummaryIndex *Index, SMDiagnostic &Err );;
    
    //llvm::MemoryBufferRef m;
    //llvm::ModuleSummaryIndex i;

    
}
