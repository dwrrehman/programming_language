//
//  main.cpp
//  interpreter
//
//  Created by Daniel Rehman on 1902096.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//


/**
 
 Manifest: ----------------------------
 
 - i want to speed of C/C++,
 - the safety of Rust,
 - the style of Swift,
 - the types of Haskell,
 - the flexibility of lisp,
 - and the readability of Python.

 --------------------------------------
 
 
 
 
 
  ------------ TODOS: for this interpreter: ---------------------------------------
 
 
    - make it so that the lien doesnt go over when we type too many characters on one line.
    - - make it so it simply creates a newline, and wraps, it, and makes it pretty.
    - - or simply allow our draw function to now draw the rest of the line.
    - - - and then we will simply scroll over the left when we have a column that is out of place.
    - - - - this might involve adding a off_set to all lines when we print them, so we only start printing from a particular offset.
    - - - that might work.
 
 

 
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <thread>
#include <codecvt>
#include <tuple>
#include <iomanip>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "color.h"
#include "terminal_manip.hpp"

// Parameters:

#define language_name   "nostril"

const std::string welcome_message = BRIGHT_GREEN "\t\t\t\t\tA " language_name " Interpreter." RESET GRAY "\n\t\t\t\t\t[By Daniel Rehman.]\n\n\t\t\t\t    (type \":help\" for more info.)" RESET;

const size_t number_of_spaces_for_tab = 4;

std::string code_prompt = BRIGHT_GREEN " ║  " RESET;
std::string command_prompt = CYAN " ╚╡ " RESET;
std::string output_prompt = GRAY "       :   " RESET;

// Globals:
std::vector<std::pair<std::string, std::string>> input = {{"", ""}};
size_t column = 0;
size_t line = 1;
bool quit = false;

// Helpers:




void process_arrow_key() {
    char direction = getch();
    
    if (direction == 65) { // up
        if (line > 0) line--;
        if (input[line - 1].first.size() < column) column = input[line - 1].first.size();
        
    } else if (direction == 66) { // down
        if (line < input.size()) line++;
        if (input[line - 1].first.size() < column) column = input[line - 1].first.size();
        
    } else if (direction == 67) { // right
        if (column < input[line - 1].first.size()) column++;
        else if (column == input[line - 1].first.size() && line < input.size()) {column = 0; line++;}
        
    } else if (direction == 68) { // left
        if (column > 0) column--;
        else if (column == 0 && line > 1) column = input[--line].first.size();
    }
}

void print_tab_representation() {
    for (size_t i = number_of_spaces_for_tab; i--;)
        input[line - 1].first.insert(column++, 1, ' ');
}

void delete_character() {
    if (column > 0) {
        input[line - 1].first.erase(input[line - 1].first.begin() + --column);
    } else if (column == 0 && line > 1) {
        input.pop_back();
        column = input[--line].first.size();
    }
}

void print_newline() {
    input[line++ - 1].first.insert(column, 1, ' ');
    input.push_back({"", ""});
    column = 0;
}

void get_input() {
    while (!quit) {
        char c = getch();
        
        if (c == '\n') {
            print_newline();
            
        } else if (c == 9) { // tab
            print_tab_representation();
            
        } else if (c == 127) { // backspace
            delete_character();
            
        } else if (c >= ' ' && c < 127) { // normal printable ascii char
            input[line - 1].first.insert(column++, 1, c);
            
        } else if (c == 27) { // escape
            char d = getch();
            if (d == 91) {
                process_arrow_key();
            }
        }
        
        if (line == 1 && column == 1 && input[line - 1].first[0] == ':') mode = command_mode;
        if (line == 1 && column == 1 && input[line - 1].first[0] != ':') mode = code_mode;
        if (line == 1 && column == 0) mode = code_mode;
        usleep(1000);
    }
}

void draw() {
    while (!quit) {
        int i = 0;
        for (auto s : input) {
            
            clear_line();
            std::cout << "\r";
            
            print_prompt(mode == command_mode ? command_prompt : code_prompt, i, line);
            std::cout << s.first;
            
            if (s.second != "") {
                std::cout << "\n";
                print_prompt(output_prompt, i, line);
                std::cout << GRAY << s.second << RESET;
            }
            i++;
        }
        move_cursor_backward((int) input[line - 1].first.size() - (int) column);
        move_cursor_upwards((int) input.size() - (int) line);
        fflush(stdout);
        
        usleep(25000);
        move_cursor_upwards((int) line - 1);
    }
}

void interpret() {
    // calls the interpreter, to interpret each statement, and output the appropriote print statement.

    while (!quit) {
        for (auto& s : input) {
            if (s.first == "pasta") {
                s.second = "linguini.";
            }
        }
        sleep(3);
    }
}

void welcome_screen() {
    clear_screen();
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    for (int i = size.ws_row/2; i--;) std::cout << "\n";
    std::cout << welcome_message << std::endl;
    getch();
    clear_screen();
}

int main() {
    welcome_screen();
    std::thread draw_thread = std::thread(draw);
    std::thread interpret_thread = std::thread(interpret);
    get_input();
    interpret_thread.join();
    draw_thread.join();
    return 0;
}
