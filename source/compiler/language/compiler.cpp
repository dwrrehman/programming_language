//
//  compiler.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <vector>

#include "compiler.hpp"
#include "preprocessor.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "analysis.hpp"

static void print_errors(bool a_error, bool l_error, bool p_error, bool t_error) {
    std::cout << "total errors in all stages:\n";
    std::cout << std::boolalpha;
    std::cout << "\t - preprocessing = " << p_error << std::endl;
    std::cout << "\t - lexing = " << l_error << std::endl;
    std::cout << "\t - parsing = " << t_error << std::endl;
    std::cout << "\t - analysis = " << a_error << std::endl;
}



struct action_tree frontend(struct file file) {
    bool p_error = false, l_error = false, t_error = false, a_error = false;
    
    auto p = preprocess(file.data, p_error);
    auto l = lex(p, l_error);
    auto t = parse(file.name, file.data, l, t_error);
    auto a = analyze(t, a_error);
    
    print_errors(a_error, l_error, p_error, t_error);
    
    return {};
}

void code_generation(struct action_tree tree) {
    
}
