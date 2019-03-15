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

#include <sstream>
#include <iostream>
#include <vector>

program parse_program() {
    // unimplemented
    return {};
}

program parse(std::string filename, std::string text) {
    start_lex(filename, text);
    
    program tree = parse_program();
    if (tree.error) { 
        return {};
    }
    return tree;
}
