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

struct expression_list;
struct expression;
struct symbol;

enum class symbol_type {
    none,    
    subexpression,    
    string_literal,
    llvm_literal,    
    identifier,    
    newline,
    indent, 
};

// literals:
struct string_literal { 
    struct token literal = {}; 
    bool error = 0;
};

struct llvm_literal { 
    struct token literal = {}; 
    bool error = 0;
};

struct identifier { 
    struct token name = {}; 
    bool error = 0;
};

struct expression_list {
    std::vector<expression> expressions = {};
    bool error = {};
    
    expression_list(){}
    expression_list(std::vector<expression> es) {
        expressions = es;
    }
    expression_list(std::vector<expression> es, bool e) {
        expressions = es;
        error = e;
    }
};

struct expression {
    std::vector<symbol> symbols = {};
    size_t indent_level = 0;
    size_t type = 0;
    llvm::Type* llvm_type = nullptr;
    bool error = false;        
    
    expression() {}
    expression(std::vector<symbol> symbols) {
        this->symbols = symbols;
    }
    expression(std::vector<symbol> symbols, size_t type) {
        this->symbols = symbols;
        this->type = type;
    }
    expression(size_t type) {        
        this->type = type;
    }
    expression(bool error, bool _ignore_me) {
        this->error = error;
    }
};

struct symbol {    
    enum symbol_type type = symbol_type::none;    
    expression_list expressions = {};
    string_literal string = {};
    llvm_literal llvm = {};
    identifier identifier = {};
    bool error = false;
    
    symbol(){}
    symbol(enum symbol_type type) {
        this->type = type;
    }
    symbol(std::string given_name) {
        this->type = symbol_type::identifier;
        this->identifier.name.value = given_name;
        this->identifier.name.type = token_type::identifier;
    }
    symbol(expression_list expressions) {
        this->type = symbol_type::subexpression; 
        this->expressions = expressions;
    }            
};







//struct expression_list {
//    std::vector<expression> expressions = {};    
//    expression_list() {}
//    expression_list(std::vector<expression> es) {
//        expressions = es;
//    }
//};




/// ---------------------- Abstractions ---------------------------



/// Delete me:

//class abstraction_definition: public node {
//public:
//    
//    enum class associativity {left, right};
//    
//    expression call_signature = {};
//    expression return_type = {};
//    std::vector<expression> body = {};
//        
//    size_t precedence = 0;
//    enum associativity associativity = associativity::left;     
//    
//    abstraction_definition(){}
//    abstraction_definition(expression call, expression rt, expression_list body) {
//        this->call_signature = call;
//        this->return_type = rt;
//        this->body = body;
//    }
//};



#endif /* nodes_hpp */
