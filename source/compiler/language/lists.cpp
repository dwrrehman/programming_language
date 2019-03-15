//
//  lists.cpp
//  language
//
//  Created by Daniel Rehman on 1901137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "lists.hpp"




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




// ------------------- nostril language keywords ------------------------------

std::vector<std::string> non_overridable_keywords = {
    "(", ")", "{", "}", ":", ".", "\\", ";", "_",
};

std::vector<std::string> keywords = {
    "_"
};

std::vector<std::string> operators = {
    "(", ")", "[", "]", "{", "}",
    "+", "-", "*", "/", "<", ">",
    "^", "%", "!", "~", "&", ".",
    "?", ":", ";", "|", "=", "\n",
    "\\", ",", "#", "$", "@",
};


// ----------------------- compiler built-in identifiers --------------------------

const std::string builtin_prefix = "__n__";

std::vector<std::string> builtins = { // (requires the prefix)

    "set_signature_visible_to",
    "visibility_all__",
    "visibility_none__",
    "visibility_to_file__",
    "visibility_to_module__",
    "visibility_group",

    "bring_signature_into_scope",
    "include_module_from_path",
    
    "set_evaluation",
    "marked_explicit_runtime__",
    "marked_explicit_compiletime__",
    "marked_implicit_compiletime__",

    "set_precedence",
    "set_associativity",
    "set_relative_precedence_from",

    //todo: add the parser AST nodes, to allow recognition of them.

    // example: __core__expression_node__, __core__statement_node__, etc.
};
