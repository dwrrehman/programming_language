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

std::string get_file(std::string filepath) {
    std::ifstream file {filepath};
    std::string text {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    if (text == "") {
        std::cout << "could not find file, file was empty: " << filepath << std::endl;
        exit(1);
    }
    return text;
}

void frontend(std::string text) {
    auto p_error = false;
    auto l_error = false;
    auto t_error = false;
    auto a_error = false;
    auto p = preprocess(text, p_error);
    auto l = lex(p, l_error);
    auto t = parse(l, t_error);
    auto a = analyze(t, a_error);
    
    
    std::cout << "total errors in all stages:\n";
    std::cout << std::boolalpha;
    std::cout << "\t - preprocessing = " << p_error << std::endl;
    std::cout << "\t - lexing = " << l_error << std::endl;
    std::cout << "\t - parsing = " << t_error << std::endl;
    std::cout << "\t - analysis = " << a_error << std::endl;
}
