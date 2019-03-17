//
//  error.hpp
//  language
//
//  Created by Daniel Rehman on 1902192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef error_hpp
#define error_hpp

#include "lexer.hpp"
#include <string>
#include <vector>

// messagers:

void print_note(std::string message);
void print_info_message(std::string filename, std::string message, size_t line, size_t column);
void print_warning_message(std::string filename, std::string message, size_t line, size_t column);
void print_error_message(std::string filename, std::string message, size_t line, size_t column);

// specialized:

void print_lex_error(std::string filename, std::string state_name, size_t line, size_t column);
void print_parse_error(std::string filename, size_t line, size_t column, std::string type, std::string found, std::string expected);


// source printers:

void print_end_of_source_code(std::string text, std::string message);
void print_source_code(std::string text, std::vector<struct token> offending_tokens);

#endif /* error_hpp */
