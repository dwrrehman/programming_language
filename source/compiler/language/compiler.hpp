//
//  compiler.hpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef compiler_hpp
#define compiler_hpp

#include "analysis.hpp"
#include "arguments.hpp"
#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

llvm::Module* frontend(struct file file);

#endif /* compiler_hpp */
