//
//  error.cpp
//  language
//
//  Created by Daniel Rehman on 1902192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "error.hpp"

///TODO: fill me.


/*
node parse(std::string text, std::vector<struct token> tokens, bool &error) {
    print_lex(tokens);
    node tree = {};
    bool parse_successful = false;
    
    if (!program(tokens, tree) || pointer != tokens.size()) {
        std::cout << "Error: parsing failed." << std::endl;
        
        int i = 0;
        for (auto n : deepest_stack_trace) {
            print_node(n, i++);
        }
        
    } else parse_successful = true;
    
    print_parse(tree);
    std::cout << "level = " << level << "\n";
    
    if (parse_successful && !level) {
        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "\n\n\n\t\tsuccessfully parsed.\n\n\n" << std::endl;
        std::cout << "------------------------------------------------" << std::endl;
    } else {
        std::cout << "\n\n\n\t\tPARSE FAILURE.\n\n\n\n" << std::endl;
        
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
            
            std::cout << "\t" << GRAY << t.line + offset << RESET GREEN "  |  " RESET << lines[index] << std::endl;
            
            if (!offset) {
                std::cout << "\t";
                for (int i = 0; i < t.column + 5; i++) std::cout << " ";
                std::cout << BRIGHT_RED << "^";
                if (t.value.size() > 1) for (int i = 0; i < t.value.size() - 1; i++) std::cout << "~";
                std::cout << RESET << std::endl;
            }
        }
        
        std::cout << std::endl;
        error = true;
    }
    
    return tree;
}
*/
