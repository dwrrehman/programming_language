 //
//  lists.cpp
//  language
//
//  Created by Daniel Rehman on 1901137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "lists.hpp"



const std::string language_name = "n3zqx2l";
const std::string language_version = "0.0.1";


size_t spaces_count_for_indent = 4;

// ------------------- preprocessor langugae keywords ---------------------------

std::vector<std::string> pp_keywords = {
    "replace", "with", "begin", "end",
    "if", "while", "else", "do", "let",
    "call", "define", "emit",
    "int", "!", "&", "|",
    "==", "<", ">", "=", ",",
    "+", "-", "/", "*", "(", ")",
    
    "\\", // not actually a keyword.
};



// ------------------- main language keywords ------------------------------

std::vector<std::string> non_overridable_operators = {
    "(", ")", "{", "}", ":"
};

std::vector<std::string> operators = {
    "(", ")", "[", "]", "{", "}",
    "+", "-", "*", "/", "<", ">",
    "^", "%", "!", "~", "&", ".",
    "?", ":", ";", "|", "=", "\n",
    "\\", ",", "#", "$", "@"
};

std::vector<std::string> builtins = {
    "__visibility", "__within", "__called", "__when",
    "__none__", "__scope__", "__self__", "__parent__",

    "__type__", "__infered__"
    "__caller__", "__file__", "__module__", "__all__",
    "__bring", "__import",

    "__evaluation", "__compiletime__", "__runtime__",
    "__precedence", "__associativity", "__left__", "__right__",

    "__after", "__before", "__inside",
    "__accept",

    "__translation_unit__",
    "__expression__", "__expression_list__",
    "__symbol", "__string_literal__", "__character_literal__", "__documentation__", "__llvm__"
    "__identifier__",
    "__number__",
    "__intrinsic__",

    // {this list is unfinished}
};
