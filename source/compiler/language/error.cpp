//
//  error.cpp
//  language
//
//  Created by Daniel Rehman on 1902192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "error.hpp"

///TODO: fill me.


/**
 


std::cout << "Expected a \"" << deepest_node.name << "\", Found a \"" << tokens[deepest_pointer - 1].value;
std::cout << "\", of type: " << convert_token_type_representation(tokens[deepest_pointer - 1].type) << std::endl << std::endl;

auto & t = tokens[deepest_pointer - 1];
std::vector<int> offsets = {-2, -1, 0, 1, 2};
std::string line = "";
std::istringstream s {text};
std::vector<std::string> lines = {};
while (std::getline(s, line)) lines.push_back(line);

for (auto offset : offsets) {
    size_t index = 0;
    if ((int) t.line - 1 + offset >= 0 && (int) t.line - 1 + offset < lines.size()) {
        index = t.line - 1 + offset;
    } else continue;
    std::cout << "\t" << t.line << "  |  " << line[index] << std::endl;
    if (!offset) {
        for (int i = 0; i < t.column + 5; i++) {
            std::cout << " ";
        }
        std::cout << "^";
        if (t.value.size() > 1) {
            for (int i = 0; i < t.value.size(); i++) {
                std::cout << "~";
            }
        }
        std::cout << std::endl;
    }
}
error = true;
}
 */
