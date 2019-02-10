//
//  parser.hpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef parser_hpp
#define parser_hpp

#include <string>
#include <vector>
#include "lexer.hpp"

class node {
public:
    std::string name = "";
    bool success = false;
    struct token data = {"", null_type, 0, 0};
    std::vector<node> children = {};
    node(std::string name, struct token data, std::vector<node> children, bool success) {
        this->name = name;
        this->children = children;
        this->success = success;
        
        this->data.type = data.type;
        this->data.value = data.value;
        
        this->data.line  = data.line;
        this->data.column = data.column;
        
    }
    node(){}
};

node parse(std::vector<struct token> tokens);

#endif /* parser_hpp */
