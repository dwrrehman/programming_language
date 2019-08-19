    //
//  nodes.hpp
//  language
//
//  Created by Daniel Rehman on 1902284.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef nodes_hpp
#define nodes_hpp


#include "lexer.hpp"
#include "llvm/IR/Type.h"
#include <vector>


class node {    // TODO: delete me
public:
    bool error = true;
};

// enum classes:

enum class symbol_type {
    none,
    subexpression,
    string_literal,
    llvm_literal,
    block,
    identifier,
    newline,
    indent,
    abstraction_definition,
};

// prototypes:

class translation_unit;
class block;
class expression_list;
class expression;
class symbol;

// literals:

class string_literal: public node {
public:
    struct token literal = {};
};

class llvm_literal: public node {
public:
    struct token literal = {};
};

class identifier: public node {
public:
    struct token name = {};
};

class expression: public node {
public:
    
    std::vector<symbol> symbols = {};
    size_t indent_level = 0;
    expression* type = nullptr;
    
    bool erroneous = false;
    bool was_allocated = false;
    
    llvm::Type* llvm_type = nullptr;
    
    expression() {}
    expression(std::vector<symbol> symbols){
        this->symbols = symbols;
    }
    expression(std::vector<symbol> symbols, expression* type) {
        this->symbols = symbols;
        this->type = type;
    }
    expression(bool error) {
        this->erroneous = error;
    }
};

class expression_list: public node {
public:
    std::vector<expression> expressions = {};
};

class translation_unit: public node {
public:
    expression_list list = {};
};

class block: public node {
public:
    expression_list list = {};
    block(){}
    block(expression_list list) {
        this->list = list;
    }
};



/// ---------------------- Abstractions ---------------------------

class abstraction_definition: public node {
public:
    
    enum class associativity {left, right};
    
    expression call_signature = {};
    expression return_type = {};
    block body = {};
        
    size_t precedence = 0;
    enum associativity associativity = associativity::left;     

    abstraction_definition(){}
    abstraction_definition(expression call, expression rt, block body) {
        this->call_signature = call;
        this->return_type = rt;
        this->body = body;
    }
};

class symbol: public node {
public:
    enum symbol_type type = symbol_type::none;
    expression subexpression = {};
    block block = {};
    string_literal string = {};
    llvm_literal llvm = {};
    identifier identifier = {};
    abstraction_definition abstraction = {};
    
    symbol(){}
    symbol(enum symbol_type type) {
        this->type = type;
    }
    symbol(std::string given_name, bool unused) {
        this->type = symbol_type::identifier;
        this->identifier.name.value = given_name;
        this->identifier.name.type = token_type::identifier;
    }
    symbol(expression subexpression) {
        this->type = symbol_type::subexpression;
        this->subexpression = subexpression;
    }
    symbol(abstraction_definition a) {
        this->type = symbol_type::abstraction_definition;
        this->abstraction = a;
    }
};

#endif /* nodes_hpp */
