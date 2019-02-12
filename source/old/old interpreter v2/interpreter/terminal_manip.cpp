//
//  terminal_manip.cpp
//  interpreter
//
//  Created by Daniel Rehman on 1902111.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "terminal_manip.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include "color.h"



enum display_mode mode = code_mode;

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

void print_prompt(std::string base, size_t offset, size_t line) {
    if (mode == command_mode)
        printf("      %s ", base.c_str());
    else if (mode == code_mode)
        printf(GRAY "%5lu " RESET "%s ", line + offset, base.c_str());
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
