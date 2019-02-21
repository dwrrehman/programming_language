//
//  lists.cpp
//  language
//
//  Created by Daniel Rehman on 1901137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "lists.hpp"

std::vector<std::string> keywords = {
    "return", "import",
    "using", "from",
    "break", "continue",
    "runtime", "noinline",
    "public", "private",
};

std::vector<std::string> overridable_keywords = {
    "return", "import",
    "using", "from",
    "break", "continue",
    "runtime", "noinline",
    "public", "private",
    
    "+", "-", "*", "/", "<", ">",
    "^", "%", "!", "~", "&", ".",
    "?", "|", "=", "[", "]"
};

std::vector<std::string> qualifiers =  {
    "compiletime", "runtime", "public", "private", "constructor", "destructor",
};

std::vector<std::string> builtin_types = {
    "i32", "i64"
};

std::vector<std::string> operators = {
    "(", ")", "[", "]", "{", "}",
    "+", "-", "*", "/", "<", ">",
    "^", "%", "!", "~", "&", ".",
    "?", ":", ";", "|", "=", "\n", "\\", ","
};

void sort_lists_by_decreasing_length() {
    std::sort(keywords.begin(), keywords.end(), [](std::string first, std::string second){return first.size() > second.size();});
    std::sort(qualifiers.begin(), qualifiers.end(), [](std::string first, std::string second){return first.size() > second.size();});
    std::sort(overridable_keywords.begin(), overridable_keywords.end(), [](std::string first, std::string second){return first.size() > second.size();});
}
