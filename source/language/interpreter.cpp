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

#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Module.h"

#include <iostream>


static nat output_line_number = 0;


std::vector<std::string> commands =  {
    
    "quit",
    "help",
    "clear",
    
    "hello?",
    "session",
    
    "do",
    "emit", 
    "sneeze",
};

void print_welcome_message() {
    std::cout << "a " << cBRIGHT_CYAN << language_name << cRESET << " REPL interpreter " cGRAY "(version " << language_version << ").\n" cRESET;
    std::cout << "type \"help\" for more information.\n\n";
}

std::string interpreter_prompt(nat line) {
    return " " cBRIGHT_CYAN "(" cRESET cWHITE + std::to_string(line) + cRESET cBRIGHT_CYAN ") " cRESET;
}

std::string output() {
    return " " cRED "{" cRESET cGRAY + std::to_string(output_line_number++) + cRESET cRED "} " cRESET;
}

static bool is_command(const std::string& given) {
    for (auto i : commands) {
        if (i == given) return true;
    }
    return false;
}

static bool is_quit_command(const std::string& line) {
    return line == "quit" or line == "exit" or line == "q";
}

void process_repl_command(const std::string& line, const std::string& first) {
            
    if (line == "clear") {
        std::cout << "\033[1;1H\033[2J";
        
    } else if (line == "hello?") {
        std::cout << output() << "Hello, world!\n";
                
    } else if (first == "do" and line.size() > 3) { // an example command which takes some string as input.
    
        auto text = line.substr(3);
        std::cout << "doing: \"" << text << "\"\n";
        
    } else if (line == "help") {
        std::cout << output() << "commands: \n";
        for (auto i : commands) {
            std::cout << "\t " << i << "\n";                        
        }
        std::cout << "\n";
    } else {
        std::cout << output() << "unimplemented.\n";        
    }
}

void interpret_llvm_string(const std::string& text) {
//    llvm::SMDiagnostic errors;
//    if (parse_llvm_string_as_function(text, state, errors)) {                
//    } else {
//        std::cout << "failure.\n";
//        errors.print("llvm string program:", llvm::errs());
//        abort();
//    }
}


//void analyze(expression_list unit, std::unique_ptr<llvm::Module>& module, struct file file) {
//    // code this after we get the regular one finished.
//}

static std::string get_first_word(const std::string& line) {
    auto first = line;
    nat i = 0;
    while (first[i] != ' ' and first[i] != 0) i++;
    first.erase(i);
    return first;
}

void repl(llvm::LLVMContext& context) {
    print_welcome_message();
    
    auto module = llvm::make_unique<llvm::Module>("_REPL", context);
    
    std::string line = "";
    nat line_number = 0;
    do {
        std::cout << interpreter_prompt(line_number);
        std::getline(std::cin, line);
        if (line != "") line_number++;
        if (is_quit_command(line)) break;
        std::string first = get_first_word(line);
        if ((line.size() and is_command(first))) {
            process_repl_command(line, first); 
            
        } else { // resolve and process expression:
            if (line.size() > 10)
                std::cout << output() << "recevied: :::" << line << ":::\n";
        }
    } while (std::cin.good());
    
    exit(0);
}

