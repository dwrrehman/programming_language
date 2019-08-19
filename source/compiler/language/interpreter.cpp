//
//  interpreter.cpp
//  language
//
//  Created by Daniel Rehman on 1902262.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "interpreter.hpp"

#include "color.h"
#include "arguments.hpp"
#include "lists.hpp"
#include "nodes.hpp"
#include "llvm/ExecutionEngine/daniels_interpreter/MCJIT.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <iostream>


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

static bool is_quit_command(const std::string &line) {
    return line == ":quit" or line == "quit" or line == ":exit" or line == "exit" or line == ":q";
}

void process_repl_command(std::string line) {
    if (line == "clear") {
        std::cout << "\e[1;1H\e[2J";
    } else if (line == "hello") {
        std::cout << output() << "Hello, world!\n";
    } else if (line == "help") {
        std::cout << output() << "REPL commands: \n\t- :clear\n\t- :hello\n\t- :help\n\t- :quit\n\n";
    }
}

void analyze(translation_unit unit, std::unique_ptr<llvm::Module>& module, struct file file) {
    // code this after we get the regular one finished.
}

void repl(llvm::LLVMContext& context) {
    print_welcome_message();
    
    auto module = llvm::make_unique<llvm::Module>("_REPL", context);
    
    std::string line = "";
    size_t line_number = 0;
    do {
        std::cout << interpreter_prompt(line_number);
        std::getline(std::cin, line);
        if (line != "") line_number++;

        if ((line.size() and line[0] == ':') or is_quit_command(line)) {
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
    exit(0);
}

