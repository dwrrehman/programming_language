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
    "(", ")", "{", "}" // and ":", but thats implicit.
};

std::vector<std::string> operators = {
    "(", ")", "[", "]", "{", "}",
    "+", "-", "*", "/", "<", ">",
    "^", "%", "!", "~", "&", ".",
    "?", ":", ";", "|", "=", "\n",
    "\\", ",", "#", "$", "@"
};

