//
//  lists.cpp
//  language
//
//  Created by Daniel Rehman on 1901137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "lists.hpp"

const std::string bulitin_prefix = "__nostril_core__";


std::vector<std::string> pp_keywords = {
    "replace", "with", "begin", "end",
    "if", "while", "else", "do", "let",
    "call", "define", "emit",
    "int", "!", "&", "|",
    "==", "<", ">", "=", ",",
    "+", "-", "/", "*", "(", ")",
    
    "\\", // not actually a keyword.
};


std::vector<std::string> non_overridable_keywords = {
    "(", ")", "{", "}", ":", ".", "\\", "__", ";",
};

std::vector<std::string> builtins = { // apply the prefix to get the real builtin identifiers.
    "type_i1__",
    "type_i8__",
    "type_i16__",
    "type_i32__",
    "type_i64__",
    "type_i128__",
    
    "type_r16__",
    "type_r32__",
    "type_r64__",
    "type_r80__",
    "type_r128__",
    
    "compiler_api__",
    
    "set_signature_visible_to",
    "visibility_all__",
    "visibility_none__",
    "visibility_file__",
    "visibility_module__",
    "visibility_to_group__",
    
    "set_inline_ability_to",
    "inline_always__",
    "inline_never__",
    "inline_if_beneficial__",
    
    "use_signature_in_scope",
    "include_module_from_path",
    
    "set_time_marking",
    "marked_explicit_runtime__",
    "marked_explicit_compiletime__",
    "marked_implicit_compiletime__",
    
    "set_linkage",
    "set_calling_convention",
    
    
    "set_precedence",
    "set_relative_precedence_from",
    
    "",
    "",
  
};

std::vector<std::string> keywords = {
    "__", "_"
};

std::vector<std::string> operators = {
    "(", ")", "[", "]", "{", "}",
    "+", "-", "*", "/", "<", ">",
    "^", "%", "!", "~", "&", ".",
    "?", ":", ";", "|", "=", "\n",
    "\\", ",", "#", "$", "@",
};
