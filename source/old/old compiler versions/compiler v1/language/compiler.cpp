//
//  compiler.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <fstream>

#include "compiler.hpp"
#include "preprocessor.hpp"
#include "lexer.hpp"
#include "parser.hpp"

std::string get_file(std::string filepath) {
    std::ifstream file {filepath};
    std::string text {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    if (text == "") {
        std::cout << "could not find file: " << filepath << std::endl;
        exit(1);
    }
    return text;
}

void frontend(std::string text) {
    auto p = preprocess(text);
    auto l = lex(p);
    auto t = parse(l);
}
