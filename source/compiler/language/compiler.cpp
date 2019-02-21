//
//  compiler.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "llvm/IR/LLVMContext.h"

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

void frontend(std::string filename, std::string text) {
    bool p_error = false, l_error = false, t_error = false, a_error = false;
    
    auto p = preprocess(text, p_error);
    auto l = lex(p, l_error);
    auto t = parse(filename, text, l, t_error);
    auto a = analyze(t, a_error);
    
    std::cout << "total errors in all stages:\n";
    std::cout << std::boolalpha;
    std::cout << "\t - preprocessing = " << p_error << std::endl;
    std::cout << "\t - lexing = " << l_error << std::endl;
    std::cout << "\t - parsing = " << t_error << std::endl;
    std::cout << "\t - analysis = " << a_error << std::endl;
}

void backend() {
    
}
