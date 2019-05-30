//
//  interpreter.hpp
//  language
//
//  Created by Daniel Rehman on 1902262.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef interpreter_hpp
#define interpreter_hpp

#include <string>
#include "llvm/IR/Module.h"

#include "llvm/IR/LLVMContext.h"

void repl(llvm::LLVMContext& context);

#endif /* interpreter_hpp */
