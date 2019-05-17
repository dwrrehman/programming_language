//
//  parser.hpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef parser_hpp
#define parser_hpp

#include "nodes.hpp"
#include "arguments.hpp"

#include "llvm/IR/LLVMContext.h"
#include <string>

translation_unit parse(struct file text, llvm::LLVMContext& context);
expression parse_expression(struct file file, bool can_be_empty, bool newlines_are_a_symbol);

#endif /* parser_hpp */

