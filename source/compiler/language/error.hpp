//
//  error.hpp
//  language
//
//  Created by Daniel Rehman on 1902192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef error_hpp
#define error_hpp

#include <string>
#include <vector>
#include "lexer.hpp"

void print_parse_error(std::string filename, size_t line, size_t column, enum token_type type, std::string found);

void print_source_code(std::string text, std::vector<struct token> tokens);




#endif /* error_hpp */
