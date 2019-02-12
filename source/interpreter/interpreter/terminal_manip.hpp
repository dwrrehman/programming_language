//
//  terminal_manip.hpp
//  interpreter
//
//  Created by Daniel Rehman on 1902111.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef terminal_manip_hpp
#define terminal_manip_hpp

#include <string>

enum display_mode {code_mode, command_mode, shell_mode};

extern enum display_mode mode;

void move_cursor_forwards(int amount);
void move_cursor_backwards(int amount);
void move_cursor_upwards(int amount);
void move_cursor_downwards(int amount);
void print_prompt(std::string base, size_t offset, size_t line);
void clear_screen();
void clear_line();
char getch();

#endif /* terminal_manip_hpp */
