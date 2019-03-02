//
//  lists.cpp
//  language
//
//  Created by Daniel Rehman on 1901137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "lists.hpp"

std::vector<std::string> non_overridable_keywords = {
    "(", ")", "{", "}", ":", ".", "\\", "_"
};

std::vector<std::string> builtin_types = {
    "i1", "i8", "i16", "i32", "i64", "i128",
    "r16", "r32", "r64", "r80","r128",
};

std::vector<std::string> keywords = {
    "_"
};

std::vector<std::string> operators = {
    "(", ")", "[", "]", "{", "}",
    "+", "-", "*", "/", "<", ">",
    "^", "%", "!", "~", "&", ".",
    "?", ":", ";", "|", "=", "\n",
    "\\", ",", "#", "$",
};
