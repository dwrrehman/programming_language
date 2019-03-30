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

void interpreter(struct file file);

std::string interpret_llvm(std::unique_ptr<llvm::Module> module);

#endif /* interpreter_hpp */
