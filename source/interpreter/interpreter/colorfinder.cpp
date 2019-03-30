//
//  colorfinder.cpp
//  interpreter
//
//  Created by Daniel Rehman on 1903181.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "colorfinder.hpp"

#include <iostream>

#define normal_color "\033[38;5;%dm"
#define reset_color "\033[0m"
#define bold_color "\033[1;38;5;%dm"


void color_finder_cli() {
    std::cout << "this is the color finder cli.\n";
    std::cout << "for more info, type \"?\".\n";
    std::string input = "";

    while (input != "q") {

        std::cout << ":color:> ";
        std::cin >> input;

        if (input == "p") {

            int bold = 0, color = 0;
            std::cin >> bold;
            std::cin >> color;

            if (bold) printf(bold_color "   bold hello   " reset_color "\n", color);
            else printf(normal_color "   hello color  " reset_color "\n", color);

        } else if (input == "?") {
            std::cout << "commands:\n    q - quit the cli.\n    p <bold_bool(0or1)> <color_int(0..255)> - print a color.\n    ? - this help menu.\n\n";

        } else {
            std::cout << "?" << std::endl;
        }
    }
}
