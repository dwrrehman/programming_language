//
//  parser.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//


#include "parser.hpp"
#include "nodes.hpp"
#include "lexer.hpp"
#include "lists.hpp"
#include "error.hpp"

#include <sstream>
#include <iostream>
#include <vector>

#define prep(_level) for (int i = _level; i--;) std::cout << ".   "

void print_node(node &self, int level) {
    prep(level); std::cout << self.name << " (" << self.children.size() << ")" << std::endl;
    if (self.data.type != null_type) {
        prep(level); std::cout << "type = " << convert_token_type_representation(self.data.type) << std::endl;
    }
    if (self.data.value != "") {
        prep(level); std::cout << "value = " << (self.data.value == "\n" ? "\\n" : self.data.value) << std::endl;
    }
    int i = 0;
    for (auto childnode : self.children) {
        std::cout << std::endl;
        if (self.children.size() > 1) {prep(level+1); std::cout << "child #" << i++ << ": " << std::endl;}
        print_node(childnode, level+1);
    }
}

void print_token(struct token t) {
    std::cout << "Error at token: \n\n";
    std::cout << "\t\t---------------------------------\n";
    std::cout << "\t\tline " << t.line << "," << t.column << " : "<< t.value << "           "  <<  "(" << convert_token_type_representation(t.type) << ")\n";
    std::cout << "\t\t---------------------------------\n\n\n";
}

void print_parse(node &tree) {
    std::cout << "------------ PARSE: ------------- " << std::endl;
    print_node(tree, 0);
}




class program parse(std::string filename, std::string text, std::vector<struct token> tokens, bool &error) {
    
    

    return {};
}
