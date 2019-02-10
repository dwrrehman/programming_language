//
//  lexer.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <vector>
#include <iostream>
#include <iomanip>
#include "lists.hpp"
#include "lexer.hpp"


void print_lex(const std::vector<struct token> &tokens) {
    std::cout << "::::::::::LEX:::::::::::" << std::endl;
    for (auto token : tokens) {
        //if (token.value == "\n") continue;
        std::cout << "TOKEN(type: " << convert_token_type_representation(token.type) << ", value: \"" << token.value << "\", [" << token.line << ":" << token.column << "])" << std::endl;
    }
    std::cout << ":::::::END OF LEX:::::::" << std::endl;
}

std::vector<struct token> lex(std::string text, bool debug) {
    if (debug)
        std::cout << "preprocessed text:::\n" << text << ":::" << std::endl;
    
    sort_lists_by_decreasing_length();
    
    const size_t text_length = text.size();
    for (int i = 0; i < keywords[0].size() + 1; i++)
        text.push_back(' ');
    
    std::vector<struct token> tokens = {};
    std::vector<bool> states = {false, false, false};
    std::vector<std::string> strings = {"", "", ""};
    
    size_t line = 1, column = 0;
    
    for (int c = 0; c < text_length; c++) {
        column++;
        
        const char first_char = text[c];
        const char second_char = text[c + 1];
        
        
        
        if (first_char == '\n') {
            line++;
            column = 0;
        }
    }
    
    return tokens;
}

