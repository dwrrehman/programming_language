//
//  error.cpp
//  language
//
//  Created by Daniel Rehman on 1902192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "error.hpp"
#include "lexer.hpp"
#include "color.h"

#include <sstream>
#include <iostream>
#include <string>
#include <vector>

void print_parse_error(std::string filename, size_t line, size_t column, enum token_type type, std::string found) {
    std::cout << "\n" << filename << ": " << line << ":" << column << " : " BRIGHT_RED "error" RESET ": unexpected " << convert_token_type_representation(type) << ", \"" << (found == "\n" ? "newline" : found) << "\"" << std::endl << std::endl;
}

void print_source_code(std::string text, std::vector<struct token> tokens) {
    auto& t = tokens[0];
    std::vector<int> offsets = {-2, -1, 0, 1, 2};
    std::string line = "";
    std::istringstream s {text};
    std::vector<std::string> lines = {};
    while (std::getline(s, line)) lines.push_back(line);
    
    for (auto offset : offsets) {
        
        size_t index = 0;
        if ((int) t.line - 1 + offset >= 0 && (int) t.line - 1 + offset < lines.size()) {
            index = t.line - 1 + offset;
        } else continue;
        
        std::cout << "\t" << GRAY << t.line + offset << RESET GREEN "  |  " RESET << lines[index] << std::endl;
        
        if (!offset) {
            std::cout << "\t";
            for (int i = 0; i < t.column + 5; i++) std::cout << " ";
            std::cout << BRIGHT_RED << "^";
            if (t.value.size() > 1) for (int i = 0; i < t.value.size() - 1; i++) std::cout << "~";
            std::cout << RESET << std::endl;
        }
    }
    
    std::cout << std::endl << std::endl;
}

