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

// main language entities:
extern std::vector<std::string> operators;
extern std::vector<std::string> non_overridable_operators;

// preprocessor entities:
extern std::vector<std::string> pp_keywords;

#endif /* lists_hpp */
