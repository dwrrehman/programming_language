//
//  lists.cpp
//  language
//
//  Created by Daniel Rehman on 1901137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "lists.hpp"

std::vector<std::string> keywords = {
    "null", "nothing", "anything",
    "real", "int", "signed", "string", "character",
    "return", "using", "use", "from", "in", "go", "to", "by",
    "break", "continue", "for", "while", "if", "else", "repeat",
    "and", "not", "or", "xor",
    "compiletime", "runtime", "pure", "impure",
    "public", "private", "constructor", "destructor", "on copy", "on move",
    "main"
};

std::vector<std::string> overridable_keywords = {
    "null", "nothing", "anything",
    "real", "int", "signed", "string", "character",
    "return", "using", "use", "from", "in", "go", "to", "by",
    "break", "continue", "for", "while", "if", "else", "repeat",
    "and", "not", "or", "xor",
    "compiletime", "runtime", "pure", "impure",
    "public", "private", "constructor", "destructor", "on copy", "on move",
    "main"
};

std::vector<std::string> qualifiers =  {
    "compiletime", "runtime", "pure", "impure",
    "public", "private", "constructor", "destructor", "on copy", "on move",
};

std::vector<std::string> builtin_types = {
    "real", "int", "signed", "string", "character",
};

std::vector<std::string> operators = {
    "(", ")", "[", "]", "{", "}",
    "+", "-", "*", "/", "<", ">",
    "^", "%", "!", "~", "&", ".",
    "?", "\n", "\\", ":", ";", "|", "=",
};


void sort_lists_by_decreasing_length() {
    std::sort(keywords.begin(), keywords.end(), [](std::string first, std::string second){return first.size() > second.size();});
    std::sort(operators.begin(), operators.end(), [](std::string first, std::string second){return first.size() > second.size();});
}
