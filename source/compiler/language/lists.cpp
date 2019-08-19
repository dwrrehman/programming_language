 //
//  lists.cpp
//  language
//
//  Created by Daniel Rehman on 1901137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "lists.hpp"

const std::string language_name = "n3zqx2l";
const std::string language_version = "0.0.01";

const std::vector<std::string> language_syntax = {"(", ")", "{", "}"};     // TODO: we need to get rid of this... its just dumb now. 

size_t spaces_count_for_indent = 4;
size_t max_expression_depth = 8;
bool debug = false;
