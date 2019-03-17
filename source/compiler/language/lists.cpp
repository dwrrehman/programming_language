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

/*
 __visibility
 __within
 __called
 __when
 __none__
 __scope__
 __self__
 __parent__
 __caller__
 __file__
 __module__
 __all__

 __bring
 __import

 __evaluation
 __compiletime__
 __runtime__

 __precedence                      ;; can be absolute, if user just passed a number, else relative.
 __first__                   ; ie, -inf
 __last__                    ; ie, +inf

 __associativity
 __left__
 __right__

 --------- some new stuff that was added -------------

 __after
 __before
 __inside

 __accept
 __expression__              ; this will contain ALL ebnf grammar nodes.
 __statement__
 __program__
 __identifier__
 __number__
 ...
 __intrinsic__                      ; ?!??!?!! what??!!? how



 */


std::vector<std::string> builtins = { // (requires the prefix)

    "__visibility", "__within", "__called", "__when",
    "__none__", "__scope__", "__self__", "__parent__",
    "__caller__", "__file__", "__module__", "__all__",
    "__bring", "__import",
    "__evaluation", "__compiletime__", "__runtime__",
    "__precedence", "__first__", "__last__",
    "__associativity", "__left__", "__right__",
    "__after", "__before", "__inside",
    "__accept",

    "__program__",
    "__expression__",
    "__statement__",
    "__identifier__",
    "__number__",
    "__intrinsic__", "{this list is unfinished}",
};
