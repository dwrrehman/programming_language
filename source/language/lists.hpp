//
//  lists.hpp
//  language
//
//  Created by Daniel Rehman on 1901137.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef lists_hpp
#define lists_hpp

#include <cstdint>
#include <vector>
#include <string>

using nat = int_fast64_t;

// language info
extern const std::string language_name;
extern const std::string language_version;

extern const std::string compiler_name;
extern const std::string compiler_version;

extern nat spaces_count_for_indent;
extern nat max_expression_depth;
extern bool debug;

#endif /* lists_hpp */
