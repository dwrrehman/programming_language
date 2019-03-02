//
//  parser.hpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef parser_hpp
#define parser_hpp

#include "lexer.hpp"

#include <string>
#include <vector>


class node;

/// Signature post info:

struct signature_element {
    std::string name = "";
    bool is_parameter = false;
    node* type = nullptr;
};

struct signature {
    std::vector<struct signature_element> signature = {};
    node* return_type = nullptr;
    size_t precedence = 0;
};


// Expression post info:

class expression_node {
public:
    
    std::string name = "";
    struct token data = {};
    std::vector<expression_node> children = {};
    
    expression_node(std::string name, struct token data, std::vector<expression_node> children) {
        this->name = name;
        this->data = data;
        this->children = children;
    }
    
    expression_node(){}
};

class postinformation {
public:
    bool is_parameter_variable = false;
    
    bool expression_has_been_parsed = false;
    expression_node expression = {};
    struct signature signature = {};
    
    postinformation(){}
};

/// Parser AST nodes:

class node {
public:
    
    std::string name = "";
    
    bool success = false;
    struct token data = {"", null_type, 0, 0};
    postinformation post = {};
    std::vector<node> children = {};
    
    node(std::string name, struct token data, std::vector<node> children, bool success) {
        this->name = name;
        this->children = children;
        this->success = success;
        this->data = data;
    }
    node(){}
};

class parse_error {
public:
    
    std::string expected = "";
    struct token at = {"", null_type, 0, 0};
    parse_error(std::string expected, struct token data) {
        this->expected = expected;
        this->at = data;
    }
    parse_error(){}
};

class program parse(std::string filename, std::string text, std::vector<struct token> tokens, bool &error);
void print_node(node &node, int level);

#endif /* parser_hpp */
