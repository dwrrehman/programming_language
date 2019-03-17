//
//  preprocessor.hpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef preprocessor_hpp
#define preprocessor_hpp

#include <string>
#include <vector>
#include <unordered_map>

// --------------- preprocessor structures -------------

// lexer ds:

enum pp_token_type {
    pp_null_type,
    pp_keyword_type,
    pp_text_type,
    pp_identifier_type,
    pp_ast_node_type
};

struct pp_token {
    enum pp_token_type type;
    std::string value;
    size_t line;
    size_t column;
};


class postinformation { // unused
public:
    postinformation(){}
};

// Parser ds:

class pp_node {
public:

    std::string name = "";
    int symbol_index = 0;
    bool success = false;
    struct pp_token data = {pp_null_type, "", 0, 0};
    postinformation post = {};
    std::vector<pp_node> children = {};

    pp_node(std::string name, struct pp_token data, std::vector<pp_node> children, bool success) {
        this->name = name;
        this->children = children;
        this->success = success;
        this->data = data;
    }
    pp_node(){}
};

class parse_error {
public:

    std::string expected = "";
    struct pp_token at = {pp_null_type, "",  0, 0};
    parse_error(std::string expected, struct pp_token data) {
        this->expected = expected;
        this->at = data;
    }
    parse_error(){}
};

// interpreter ds:

enum value_type {
    null_value_type,
    int_value_type,
    text_value_type,
    function_value_type
};

struct value {
    int numeric = 0;
    std::string textual = "";
    pp_node* function_definition = nullptr;
    std::unordered_map<std::string, struct value> call_scope;
    enum value_type type = null_value_type;
};


std::string preprocess(std::string text, std::string filename);

#endif /* preprocessor_hpp */
