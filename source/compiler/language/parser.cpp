//
//  parser.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "parser.hpp"

#include "lexer.hpp"
#include "nodes.hpp"
#include "lists.hpp"
#include "error.hpp"
#include "debug.hpp"

#include "llvm/IR/LLVMContext.h"

#include <sstream>
#include <iostream>
#include <vector>

/*
static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}*/


std::unique_ptr<translation_unit> parse_translation_unit() {
    debug_token_stream();
    return llvm::make_unique<translation_unit>();
}

translation_unit parse(std::string text, std::string filename) {
    start_lex(filename, text);

    auto tree = parse_translation_unit();
    if (!tree) {
        // print errors?
        return {};
    }
    return std::move(*tree);
}
