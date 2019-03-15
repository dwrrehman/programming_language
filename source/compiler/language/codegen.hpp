//
//  codegen.hpp
//  language
//
//  Created by Daniel Rehman on 1903085.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef codegen_hpp
#define codegen_hpp

#include "analysis.hpp"

#include "llvm/IR/Module.h"

llvm::Module* code_generation(struct action_tree tree);

#endif /* codegen_hpp */
