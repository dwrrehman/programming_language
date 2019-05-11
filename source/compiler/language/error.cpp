//
//  error.cpp
//  language
//
//  Created by Daniel Rehman on 1902192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "error.hpp"
#include "lexer.hpp"
#include "lists.hpp"
#include "color.h"

#include <sstream>
#include <iostream>
#include <string>
#include <vector>


// helpers:

static std::string contract_filename(std::string filename) {
    const size_t threshold_length = 30;
    if (filename.size() > threshold_length) {
        std::string shorter(std::find(filename.begin() + (filename.size() - threshold_length), filename.end(), '/'), filename.end());
        shorter.insert(0, "...");
        return shorter;
    } else return filename;
}


static std::string error_heading( const std::string &filename, size_t line, size_t column) {
    std::ostringstream s;
    std::string shorter_filename = contract_filename(filename);
    s << "\n" CYAN << language_name << RESET GRAY ": " RESET BRIGHT_RED "error" RESET GRAY ": " RESET MAGENTA << shorter_filename << RESET GRAY ":" << line << ":" << column << ": " RESET;
    return s.str();
}

static std::string warning_heading( const std::string &filename, size_t line, size_t column) {
    std::ostringstream s;
    std::string shorter_filename = contract_filename(filename);
    s << "\n" CYAN << language_name << RESET GRAY ": " RESET BRIGHT_YELLOW "warning" RESET GRAY ": " RESET MAGENTA << shorter_filename << RESET GRAY ":" << line << ":" << column << ": " RESET;
    return s.str();
}

static std::string info_heading( const std::string &filename, size_t line, size_t column) {
    std::ostringstream s;
    std::string shorter_filename = contract_filename(filename);
    s << "\n" CYAN << language_name << RESET GRAY ": " RESET BRIGHT_BLUE "info" RESET GRAY ": " RESET MAGENTA << shorter_filename << RESET GRAY ":" << line << ":" << column << ": " RESET;
    return s.str();
}

static std::string note_heading() {
    std::ostringstream s;
    s << "\n" CYAN << language_name << RESET GRAY ": \tnote: " RESET;
    return s.str();
}




// messagers:

void print_error_message(std::string filename, std::string message, size_t line, size_t column) {
    std::cerr << error_heading(filename, line, column) << message << std::endl;
}

void print_warning_message(std::string filename, std::string message, size_t line, size_t column) {
    std::cerr << warning_heading(filename, line, column) << message << std::endl;
}

void print_info_message(std::string filename, std::string message, size_t line, size_t column) {
    std::cerr << info_heading(filename, line, column) << message << std::endl;
}

void print_note(std::string message) {
    std::cerr << note_heading() << message << std::endl;
}



// specialized:

void print_lex_error(std::string filename, std::string state_name, size_t line, size_t column) {
    std::cerr << error_heading(filename, line, column) << "unterminated " << state_name << std::endl;
}

void print_parse_error(std::string filename, size_t line, size_t column, std::string type, std::string found, std::string expected) {
    if (type == "{null}" or found == "\n" or type == "indent") {

        if (type == "{null}") found = "end of file";
        if (found == "\n") found = "newline";
        if (type == "indent") found = "indent";

        std::cerr << error_heading(filename, line, column) << "unexpected " << found << ", expected " << expected << std::endl;
    } else {
        std::cerr << error_heading(filename, line, column) << "unexpected " << type << ", \"" << found << "\", expected " << expected << std::endl;
    }
}

void print_error_no_files() {
    std::cerr << WHITE << "nostril" << RESET GRAY ": " RESET BRIGHT_RED "error" RESET GRAY ": " RESET << "no input files" << std::endl;
    exit(1);
}







// source printers:

void syntax_highlight(std::string& text) {
    //TODO: fill in this function.
}

void print_source_code(std::string text, std::vector<struct token> offending_tokens) {

    syntax_highlight(text);

    auto& t = offending_tokens[0]; //TODO: allow this function to print erros to do with multiple tokens in combintation.
    std::vector<int> offsets = {-2, -1, 0, 1, 2};

    std::string line = "";
    std::istringstream s {text};
    std::vector<std::string> lines = {};
    while (std::getline(s, line)) lines.push_back(line);

    std::cout << "\n";
    for (auto offset : offsets) {
        size_t index = 0;
        if ((int) t.line - 1 + offset >= 0 and (int) t.line - 1 + offset < lines.size()) {
            index = t.line - 1 + offset;
        } else continue;
        
        std::cout << "\t" << GRAY << t.line + offset << RESET GREEN "  │  " RESET << lines[index] << std::endl;
        
        if (!offset) {
            std::cout << "\t";
            for (int i = 0; i < t.column + 5; i++) std::cout << " ";
            std::cout << BRIGHT_RED << "^";
            if (t.value.size() > 1) for (int i = 0; i < t.value.size() - 1; i++) std::cout << "~";
            std::cout << RESET << std::endl;
        }
    }
    
    std::cout << "\n" << std::endl;
}
