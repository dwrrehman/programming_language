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
    
    token literal = {}; 
    bool error = 0;
    
    string_literal(){}
    string_literal(bool e, bool _ignore_me, bool _also_ignore_me) { error = e; }
    string_literal(token t) {literal = t;}
};

struct llvm_literal { 
    
    struct token literal = {}; 
    bool error = 0;
        
    llvm_literal(){}
    llvm_literal(bool e, bool _ignore_me, bool _also_ignore_me) { error = e; }
    llvm_literal(token t) {literal = t;}
};

struct identifier { 
    
    struct token name = {}; 
    bool error = 0;
        
    identifier(){}
    identifier(bool e, bool _ignore_me, bool _also_ignore_me) { error = e; }
    identifier(token n) {name = n;}
};

struct expression_list {
    
    std::vector<expression> list = {};
    bool error = {};
        
    expression_list(){}
    expression_list(bool e, bool _ignore_me, bool _also_ignore_me) { error = e; }
    expression_list(std::vector<expression> es) {
        list = es;
    }
    expression_list(std::vector<expression> es, bool e) {
        list = es;
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
    expression(bool e, bool _ignore_me, bool _also_ignore_me) {
        error = e;
    }
};

struct symbol { 
    
    enum symbol_type type = symbol_type::none;    
    expression_list expressions = {};
    expression subexpression = {};
    string_literal string = {};
    llvm_literal llvm = {};
    identifier identifier = {};
    bool error = false;
    
    
    symbol(){}
    symbol(bool e, bool _ignore_me, bool _also_ignore_me) { error = e; }
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
    symbol(string_literal literal) {
        this->type = symbol_type::string_literal; 
        this->string = literal;
    }    
    symbol(llvm_literal literal) {
        this->type = symbol_type::llvm_literal; 
        this->llvm = literal;
    }    
    symbol(struct identifier id) { 
        this->type = symbol_type::identifier; 
        this->identifier = id;
    }
};

#endif /* nodes_hpp */
