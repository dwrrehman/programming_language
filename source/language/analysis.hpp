//
//  analysis.hpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef analysis_hpp
#define analysis_hpp

#include "arguments.hpp"
#include "nodes.hpp"
#include "analysis_ds.hpp"
#include "llvm/IR/IRBuilder.h"
#include <iostream>

std::unique_ptr<llvm::Module> analyze(expression_list unit, file file, llvm::LLVMContext& context);

#endif /* analysis_hpp */
