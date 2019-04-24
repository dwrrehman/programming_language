 //
//  lists.cpp
//  language
//
//  Created by Daniel Rehman on 1901137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "lists.hpp"

const std::string language_name = "n3zqx2l";
const std::string language_version = "0.0.2";

const std::vector<std::string> syntax = {"(", ")", "{", "}"};

size_t spaces_count_for_indent = 4;
size_t max_expression_depth = 8; // TODO: change to the default of "128" when analysis things work.
