//
//  expressions.cpp
//  language
//
//  Created by Daniel Rehman on 1902096.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "expressions.hpp"
#include "parser.hpp"
#include "lists.hpp"

#include <iostream>


/// Expression Debugger:

#define prep(_level) for (int i = _level; i--;) std::cout << ".   "

void print_expression_node(expression_node &self, int level) {
    prep(level); std::cout << self.name << std::endl;
    if (self.data.type != null_type) {
        prep(level); std::cout << "type = " << convert_token_type_representation(self.data.type) << std::endl;
    }
    if (self.data.value != "") {
        prep(level); std::cout << "value = " << self.data.value << std::endl;
    }
    int i = 0;
    for (auto child : self.children) {
        std::cout << std::endl;
        if (self.children.size() > 1) {prep(level+1); std::cout << "child #" << i++ << ": " << std::endl;}
        print_expression_node(child, level+1);
    }
}

void print_expression_parse(expression_node &tree) {
    std::cout << "----------------------- EXPRESSION PARSE: ---------------------- " << std::endl;
    print_expression_node(tree, 0);
}

void print_expression_token(struct token t) {
    std::cout << "Error at token: \n\n";
    std::cout << "\t\t---------------------------------\n";
    std::cout << "\t\tline " << t.line << ": " << t.value << "           "  <<  "(" << convert_token_type_representation(t.type) << ")\n";
    std::cout << "\t\t---------------------------------\n\n\n";
}


//static int pointer = 0;

expression_node parse_expression(node input, std::vector<struct function_signature> current_signatures) {
    
    expression_node tree = {};
    //bool parse_successful = false;
    
    return tree;
}
