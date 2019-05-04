//
//  lists.hpp
//  language
//
//  Created by Daniel Rehman on 1901137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef lists_hpp
#define lists_hpp

#include <vector>
#include <string>

// language info
extern const std::string language_name;
extern const std::string language_version;

extern size_t spaces_count_for_indent;
extern size_t max_expression_depth;
extern bool debug;

// main language syntax:
extern const std::vector<std::string> language_syntax;


#endif /* lists_hpp */
