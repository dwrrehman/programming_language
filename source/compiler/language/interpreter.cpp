//
//  interpreter.cpp
//  language
//
//  Created by Daniel Rehman on 1902262.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "interpreter.hpp"
#include "parser.hpp"
#include "color.h"
#include "lists.hpp"
#include "arguments.hpp"

#include <iostream>
#include <vector>

static size_t output_line_number = 0;

void print_welcome_message() {
    std::cout << "a " << BRIGHT_CYAN << language_name << RESET << " REPL interpreter " GRAY "(version " << language_version << ").\n" RESET;
    std::cout << "type \":help\" for more information.\n\n";
}

std::string interpreter_prompt(size_t line) {
    return "   " BRIGHT_CYAN "(" RESET WHITE + std::to_string(line) + RESET BRIGHT_CYAN ")" RESET GRAY ": " RESET;
}

std::string output() {
    return "   " BRIGHT_RED "{" RESET GRAY + std::to_string(output_line_number++) + RESET BRIGHT_RED "}" RESET GRAY ": " RESET;
}

void process_repl_command(std::string line) {
    if (line == "clear") {
        std::cout << "\e[1;1H\e[2J";
    } else if (line == "poke") {
        std::cout << output() << "Hello, world!\n";
    } else if (line == "help") {
        std::cout << output() << "this is a helpful message.\n";
    }
}

static bool is_quit_command(const std::string &line) {
    return line == ":quit" || line == "quit" || line == ":exit" || line == "exit";
}

void repl() {
    print_welcome_message();

    std::string line = "";
    size_t line_number = 0;
    do {
        std::cout << interpreter_prompt(line_number);
        std::getline(std::cin, line);
        if (line != "") line_number++;

        if ((line.size() && line[0] == ':') || is_quit_command(line)) {
            if (is_quit_command(line)) break;
            else {
                line.erase(line.begin(), line.begin() + 1);
                process_repl_command(line);
            }

        } else {
            if (line.size() > 10)
                std::cout << output() << "recevied: :::" << line << ":::\n";
        }
    } while (std::cin.good());
}

void editor(std::string text) {

    // do somethign cool, with compilation in the back ground.
    std::cout << "this is an editor! :D\n";

    // i will be calling my other editor code here.
}

void interpreter(struct file file) {
    if (file.name == "{repl}") {
        repl();
    } else {
        editor(file.data);
    }
    exit(0);
}
