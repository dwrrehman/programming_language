//
//  error.hpp
//  language
//
//  Created by Daniel Rehman on 1902192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef error_hpp
#define error_hpp

#include "lists.hpp"

#include <string>

// messagers:

void print_note(const std::string& message);
void print_info_message(const std::string& filename, const std::string& message, nat line, nat column);
void print_warning_message(const std::string& filename, const std::string& message, nat line, nat column);
void print_error_message(const std::string& filename, const std::string& message, nat line, nat column);

// specialized:

void print_lex_error(const std::string& filename, const std::string& state_name, nat line, nat column);
void print_parse_error(const std::string& filename, nat line, nat column, const std::string& type, std::string found, const std::string& expected);

void print_error_no_files();


// source printers:

//void print_end_of_source_code(const std::string& text, const std::string& message);
void print_source_code(std::string text, const std::vector<struct token>& offending_tokens);

#endif /* error_hpp */
