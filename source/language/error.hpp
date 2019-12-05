#ifndef error_hpp
#define error_hpp

#include "lists.hpp"

#include <string>

void print_warning_message(const std::string& filename, const std::string& message, nat line, nat column);
void print_error_message(const std::string& filename, const std::string& message, nat line, nat column);
void print_lex_error(const std::string& filename, const std::string& state_name, nat line, nat column);
void print_parse_error(const std::string& filename, nat line, nat column, const std::string& type, std::string found, const std::string& expected);
void print_unresolved_error(const struct expression &given, struct state &state);

#endif /* error_hpp */
