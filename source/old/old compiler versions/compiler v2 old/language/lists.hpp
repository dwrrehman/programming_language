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

extern std::vector<std::string> keywords;
extern std::vector<std::string> operators;
extern std::vector<std::string> qualifiers;
extern std::vector<std::string> overridable_keywords;
extern std::vector<std::string> builtin_types;

void sort_lists_by_decreasing_length();

#endif /* lists_hpp */
