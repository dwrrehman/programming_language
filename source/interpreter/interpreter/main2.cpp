//
//  main.cpp
//  interpreter
//
//  Created by Daniel Rehman on 1902096.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

/**
 
  ------------ TODOS: for this interpreter: ---------------------------------------
 
 - use min and max functions from std alg, instead of all these ternary operators.

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
#include <mutex>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "color.h"
#include "terminal_manip.hpp"
#include "colorfinder.hpp"

// Parameters:

#define language_name   "n3zqx2l"

const std::string welcome_message = BRIGHT_GREEN "A " language_name " Interpreter." RESET GRAY "\n[Created by Daniel Rehman.]\n(type \":help\" for more info.)" RESET;

const size_t number_of_spaces_for_tab = 4;

std::string code_prompt = BRIGHT_GREEN " ║  " RESET;       // all unused
std::string command_prompt = CYAN " ╚╡ " RESET;
std::string output_prompt = GRAY "       :   " RESET;

#define up direction == 65
#define down direction == 66
#define right direction == 67
#define left direction == 68


/// Gobals:

std::mutex input_lock;
std::vector<std::string> input = {""};
std::vector<std::vector<std::string>> output = {{}};

size_t first_line = 0;
size_t first_column = 0;

size_t column = 0;
size_t line = 0;

bool quit = false;
bool paused = false;


/// Helpers:

void update_view_position() {
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
    
    const size_t width = size.ws_col;
    const size_t height = size.ws_row;
    
    if (line > height) {
        first_line++;
    } else if (line < height && first_line) {
        first_line--;
    } else if (column > width) {
        first_column += 5;
    } else if (column < width && first_column) {
        first_column -= 5;
        if (first_column < 0) first_column = 0;
    }
}

void process_arrow_key() {
    char direction = getch();
    if (up && line) line--;
    else if (down && line < input.size() - 1) line++;
    else if (right && line < input.size() && column < input[line].size()) column++;
    else if (left && column) column--;
    update_view_position();
}

void print_character(char c) {
    if (line < input.size()) {
        column = column > input[line].size() ? input[line].size() : column;
        input[line].insert(column++, 1, c);                                              if (c == 'q') paused = true;     /// temp
    }
}

void print_tab() {
    for (size_t i = number_of_spaces_for_tab; i--;) print_character(' ');
}

void delete_character() {
    if (column && line < input.size()) {
        input[line].erase(input[line].begin() + --column);
    } else if (line > 0 && line < input.size()) {
        const std::string current_line = input[line];
        input.erase(input.begin() + line--);
        column = input[line].size();
        input[line].insert(input[line].size(), current_line);
    }
}

void print_newline() {
    if (column == input[line].size()) {
        input.insert(input.begin() + line + 1, "");
        column = 0; line++;
    } else if (column < input[line].size() && line < input.size()) {
        const std::string new_line = input[line].substr(column);
        input[line].erase(column);
        input.insert(input.begin() + ++line, new_line);
        column = 0;
    }
}

void get_input() {
    while (!quit) {
        if (paused) {sleep(1); continue;}
        char c = getch();
        input_lock.lock();
        if (c == '\n') {
            print_newline();
        } else if (c == 9) { // tab
            print_tab();
        } else if (c == 127) { // backspace
            delete_character();
        } else if (c >= ' ' && c < 127) { // normal printable ascii char
            print_character(c);
        } else if (c == 27) { // escape
            char d = getch();
            if (d == 91) process_arrow_key();
            else if (d == 27) quit = true;
            else if (d == '<') column = 0;
            else if (d == '>') column = input[line].size();
        }
        input_lock.unlock();
        usleep(1000);
    }
}

void draw() {
    while (!quit) {
        if (paused) {sleep(1); continue;}
        clear_screen();
        input_lock.lock();
        
        struct winsize size;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
        const size_t line_count = size.ws_row < input.size() ? size.ws_row : input.size();
        
        for (size_t i = first_line; i < line_count; i++) {
            std::cout << input[i].substr(first_column, input[i].size());
            if (i < line_count) std::cout << "\n";
        }
        
        const size_t displayed_column = column > input[line].size() ? input[line].size() : column;
        set_cursor((int) line + 1, (int) displayed_column + 1);
        input_lock.unlock();
        fflush(stdout);
        usleep(25000);
    }
}

void interpret() { // this function does syntax highlighting as well.
    while (!quit) {
        int i = 0;
        input_lock.lock();
        for (auto& l : input) {
            if (l == "PASTA") output[i] = {"linguini."};
            // note, the print statements always have a newline at the end, forcibly.
            i++;
        }
        input_lock.unlock();
        
        if (paused) {
            clear_screen();
            std::cout << "lines: {" << std::endl;
            for (auto s : input) {
                std::cout << "\"" << s << "\"," << std::endl;
            }
            std::cout << "}" << std::endl;
            std::cout << "line = " << line << ", col = " << column << std::endl;
            getch();
            clear_screen();
            paused = false;
        }
        
        sleep(1);
    }
}

int main() {
    color_finder_cli();
    std::cout << welcome_message << std::endl;
    getch();
    std::thread draw_thread = std::thread(draw);
    std::thread interpret_thread = std::thread(interpret);
    get_input();
    interpret_thread.detach();
    draw_thread.join();
    return 0;
}
