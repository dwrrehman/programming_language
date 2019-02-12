//
//  preprocessor.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include "preprocessor.hpp"

std::string preprocess(std::string text, bool debug) {
    
    if (debug)
        std::cout << "text:\n :::\n" << text << ":::" << std::endl;
    
    text.push_back(' ');
    
    std::string result = "";
    bool in_multi_comment = false;
    bool in_line_comment = false;
    
    for (int c = 0; c < text.size(); c++) {
        if (!in_line_comment && !in_multi_comment && text[c] == ':' && text[c+1] == ':') {
            in_line_comment = true;
            result.push_back(' ');
            
        } else if (in_line_comment && !in_multi_comment && text[c] == '\n') {
            result.push_back('\n');
            in_line_comment = false;
            
        } else if (!in_line_comment && !in_multi_comment && text[c] == '/' && text[c+1] == ':') {
            in_multi_comment = true;
            result.push_back(' ');
            
        } else if (!in_line_comment && in_multi_comment && text[c-1] == ':' && text[c] == '/') {
            in_multi_comment = false;
            result.push_back(' ');
            
        } else if (!in_line_comment && !in_multi_comment) {
            result.push_back(text[c]);
        } else if (text[c] == '\n'){
            result.push_back('\n');
        } else {
            result.push_back(' ');
        }
    }
    
    if (in_multi_comment) {
        std::cout << "Error: unterminated multiline comment." << std::endl;
        exit(1);
    }
    
    return result;
}
