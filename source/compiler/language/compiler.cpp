//
//  compiler.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "compiler.hpp"
#include "preprocessor.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "nodes.hpp"
#include "analysis.hpp"

#include <iostream>
#include <fstream>
#include <vector>

static void print_errors(bool a_error, bool l_error, bool p_error, bool t_error) {
    std::cout << "total errors in all stages:\n";
    std::cout << std::boolalpha;
    std::cout << "\t - preprocessing = " << p_error << std::endl;
    std::cout << "\t - lexing = " << l_error << std::endl;
    std::cout << "\t - parsing = " << t_error << std::endl;
    std::cout << "\t - analysis = " << a_error << std::endl;
}

struct action_tree frontend(struct file file) {
    bool preprocess_error = false, lex_error = false, parse_error = false, analysis_error = false;
    
    auto text = preprocess(file.name, file.data, preprocess_error);
    auto tokens = lex(text, lex_error);
    auto ast = parse(file.name, file.data, tokens, parse_error);
    auto action_tree = analyze(ast, analysis_error);
    
    print_errors(analysis_error, lex_error, preprocess_error, parse_error);
    
    return action_tree;
}

void code_generation(struct action_tree tree) {
    
}
