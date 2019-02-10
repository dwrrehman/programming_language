//
//  main.cpp
//  interpreter
//
//  Created by Daniel Rehman on 1902096.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//


/**
 
 
 
 
 things we need to do for this interpreter:
 
 
 fix the left right serach for large statements.
 
 instead of just blndly usionng curly brace, lets do semantic analysis on the instances of curiliyes or other braces, in the code, using a parser esc approach
.
 
 
 although, it would be more general if we were able to hook it up to the compiler FE...
 
 
 definitely for sure, though, i want to have a
 
 
 
 
 
 
 
 
 i
  want to speed fo C/C++, the safeness of Rust, the syntax of Swift, the types of Haskell, and the readability of Python.
 
 its a tall order, but its possible.
 
 
 
 
 
 

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

// Parameters:

const std::string welcome_message = BRIGHT_GREEN "\t\t\t\t\tA Lang Interpreter." RESET GRAY "\n\t\t\t\t\t[By Daniel Rehman.]\n\n\t\t\t\t    (type \":help\" for more info.)" RESET;

std::string code_prompt = BRIGHT_GREEN " ║  " RESET;
std::string command_prompt = BRIGHT_BLUE " ╚╡ " RESET;
std::string output_prompt = BRIGHT_RED "       :   " RESET;
enum display_mode {code_mode, command_mode, shell_mode};

// Globals:

std::vector<std::string> input = {""};

size_t column = 0;
size_t line = 1;
size_t current_line_number = 0;

std::vector<std::string> current_input = {""};
std::vector<std::vector<std::string>> command_history = {};
size_t history_position = 0;

bool quit = false;
enum display_mode mode = code_mode;

// Helpers:

std::string wchar_to_string(std::wstring c) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> cv;
    return cv.to_bytes(c[0]);
}

void move_cursor_backward(int amount) {
    if (amount <= 0) return;
    std::cout << "\033[" << amount << "D";
}

void move_cursor_upwards(int amount) {
    if (amount <= 0) return;
    std::cout << "\033[" << amount << "A";
}

void move_cursor_downwards(int amount) {
    if (amount <= 0) return;
    std::cout << "\033[" << amount << "B";
}

void print_prompt(std::string base, int line) {
    if (mode == command_mode)
        printf("      %s ", base.c_str());
    else if (mode == code_mode)
        printf(GRAY "%5lu " RESET "%s ", current_line_number + line, base.c_str());
}

void clear_screen() {
    std::cout << "\033[2J\033c";
}

void clear_line() {
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    std::cout << "\r";
    for (int i = 0; i < size.ws_col - 1; i++) std::cout << " ";
}

char getch() {
    struct termios t = {0}; if (tcgetattr(0, &t) < 0) perror("tcsetattr()");
    t.c_lflag &= ~ICANON; t.c_lflag &= ~ECHO; t.c_cc[VMIN] = 1; t.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &t) < 0) perror("tcsetattr ICANON");
    char c = 0; if (read(0, &c, 1) < 0) perror("read()"); t.c_lflag |= ICANON; t.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &t) < 0) perror("tcsetattr ~ICANON");
    return c;
}

void process(std::vector<std::string> input_lines) {
    
    std::cout << std::endl;
    
    const std::string input = input_lines[0];
    
    if (input == "quit" || input == "exit" || input == ":quit" || input == ":exit") {
        std::cout << output_prompt << "quitting...\n";
        quit = true;
        
    } else if (input == ":help" && mode == command_mode) {
        std::cout << output_prompt << "this is help message for this interpreter!" << std::endl;
        
    } else if (input == ":clear" && mode == command_mode) {
        clear_screen();
        
    } else if (input == "print hello world" && mode == code_mode) {
        std::cout << output_prompt << "hello world" << std::endl;

    } else if (input == "") {
    } else std::cout << output_prompt << "?" << std::endl;
    
    if (input != "") {
        command_history.push_back(input_lines);
        history_position++;
    }
}


void get_input() {
    
    bool ignore_newline = false;
    
    while (!quit) {
        
        char c = getch();
        
        if (c == '{') 
            ignore_newline = true;
        if (c == '}')
            ignore_newline = false;
            
        if (!ignore_newline && c == '\n') {
            process(input);
            current_line_number += input.size();
            input = {""};
            line = 1;
            column = 0;
            mode = code_mode;
            
        } else if (ignore_newline && c == '\n') {
            input[line++ - 1].insert(column, 1, c);
            input.push_back("");
            column = 0;
            
        } else if (c == '\t' || c == 9) {
            c = ' ';
            input[line - 1].insert(column++, 1, c);
            input[line - 1].insert(column++, 1, c);
            input[line - 1].insert(column++, 1, c);
            input[line - 1].insert(column++, 1, c);
            history_position = command_history.size();
            
        } else if (c == 127 && column > 0) {
            input[line - 1].erase(input[line - 1].begin() + --column);
            
        } else if (c == 127 && column == 0 && line > 1) {
            input.pop_back();
            column = input[--line].size();
        } else if (c >= ' ' && c < 127) {
            input[line - 1].insert(column++, 1, c);
            history_position = command_history.size();
            
        } else if (c == 27) {
             char d = getch();
            if (d == 91) {
                char direction = getch();
                
                if (direction == 65) { // up
                    if (history_position <= 0) printf("\a");
                    
                    else if (history_position == command_history.size()) {
                        current_input = input;
                        
                        for (int i = 0; i < (int) input.size() - 1; i++) {
                            clear_line();
                            move_cursor_upwards(1);
                        }
                        move_cursor_downwards((int) command_history[history_position-1].size() - 1);
                        input = command_history[--history_position];
                        line = input.size();
                        column = input[line - 1].size();
                    }
                    else {
                        for (int i = 0; i < (int) input.size() - 1; i++) {
                            clear_line();
                            move_cursor_upwards(1);
                        }
                        move_cursor_downwards((int) command_history[history_position-1].size() - 1);
                        input = command_history[--history_position];
                        line = input.size();
                        column = input[line - 1].size();
                    }
                    
                } else if (direction == 66) { // down
                    if (history_position >= command_history.size()) {
                        if (history_position == command_history.size()) {
                            for (int i = 0; i < (int) input.size() - 1; i++) {
                                clear_line();
                                move_cursor_upwards(1);
                            }
                            move_cursor_downwards((int) current_input.size() - 1);
                            input = current_input;
                            
                            line = input.size();
                            column = input[line - 1].size();
                            
                        } else printf("\a");
                    } else {
                        for (int i = 0; i < (int) input.size() - 1; i++) {
                            clear_line();
                            move_cursor_upwards(1);
                        }
                        move_cursor_downwards((int) command_history[history_position].size() - 1);
                        input = command_history[history_position++];
                        
                        line = input.size();
                        column = input[line - 1].size();}
                    
                } else if (direction == 67) { // right
                    if (column < input[line - 1].size()) column++;
                    else if (column == input[line - 1].size() && line < input.size()) {column = 0; line++;}
                    
                } else if (direction == 68) { // left
                    if (column > 0) column--;
                    else if (column == 0 && line > 1) column = input[--line].size();
                }
            }            
        }
        
        if (line == 1 && column == 1 && input[line - 1][0] == ':')
            mode = command_mode;
        if (line == 1 && column == 1 && input[line - 1][0] != ':')
            mode = code_mode;
        if (line == 1 && column == 0)
            mode = code_mode;
        
        usleep(1000);
    }
}

void draw() {
    while (!quit) {
        int i = 0;
        for (auto s : input) {
            clear_line();
            std::cout << "\r";
            print_prompt(mode == command_mode ? command_prompt : code_prompt, i++);
            std::cout << s;
        }
        move_cursor_backward((int) input[line - 1].size() - (int) column);
        move_cursor_upwards((int) input.size() - (int) line);
        fflush(stdout);
        usleep(25000);
        move_cursor_upwards((int) line - 1);
    }
}

int main(int argc, const char * argv[]) {
    
    clear_screen();
    
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    for (int i = size.ws_row/2; i--;) std::cout << "\n";
    std::cout << welcome_message << std::endl;
    getch();
    clear_screen();
    
    std::thread draw_thread = std::thread(draw);
    get_input();
    draw_thread.join();
    
    return 0;
}
