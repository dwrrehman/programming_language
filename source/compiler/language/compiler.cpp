//
//  compiler.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "compiler.hpp"
#include "arguments.hpp"
#include "preprocessor.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "nodes.hpp"
#include "analysis.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include <iostream>
#include <fstream>
#include <vector>

llvm::Module* frontend(struct file file) {
    return code_generation(analyze(parse(file.name, preprocess(file.name, file.data))));
}

