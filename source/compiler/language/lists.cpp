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
    "_visibility", "_within", "_called", "_when",
    "_none", "_scope", "_self", "_parent",

    "_type", "_infered"
    "_caller", "_file", "_module", "_all",
    "_bring", "_import",

    "_evaluation", "_compiletime__", "_runtime",
    "_precedence", "_associativity", "_left", "_right",

    "_after", "_before", "_inside",
    "_accept",

    "_translation_unit",
    "_expression", "_expression_list",
    "_symbol", "_string_literal", "_character_literal", "_documentation", "_llvm"
    "_identifier",
    "_number",
    "_intrinsic",
    // {this list is unfinished}

};

