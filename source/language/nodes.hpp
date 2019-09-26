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
#include "lists.hpp"
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
    string_literal(bool e, bool _, bool __): error(e) {}
    string_literal(token t): literal(t) {}
};

struct llvm_literal { 
    
    token literal = {}; 
    bool error = 0;
        
    llvm_literal(){}
    llvm_literal(bool e, bool _, bool __): error(e) {}
    llvm_literal(token t): literal(t) {}
};

struct identifier { 
    
    token name = {}; 
    bool error = 0;
    
    identifier(){}
    identifier(bool e, bool _, bool __): error(e) {}
    identifier(token n): name(n) {}
    identifier(std::string given_name) {        
        name.value = given_name;
        name.type = token_type::identifier;
    }
};

struct expression_list {
    
    std::vector<expression> list = {};
    bool error = {};
    struct token starting_token = {};
        
    expression_list(){}
    expression_list(bool e, bool _, bool __): error(e) {}
    expression_list(std::vector<expression> es): list(es) {}    
    expression_list(std::vector<expression> es, bool e): list(es), error(e) {}
};

struct expression {

    std::vector<symbol> symbols = {};
    nat indent_level = 0;
    nat type = 0;
    llvm::Type* llvm_type = nullptr;
    bool error = false;
    struct token starting_token = {};
    
    expression() {}
    expression(bool e, bool _, bool __): error(e) {}
    expression(std::vector<symbol> s, nat t = 0): symbols(s), type(t) {}
    expression(nat t): type(t) {}
};

struct symbol { 
    
    symbol_type type = symbol_type::none;    
    expression_list expressions = {};
    string_literal string = {};
    llvm_literal llvm = {};
    identifier identifier = {};
    bool error = false;
    
    symbol(){}
    symbol(bool e, bool _, bool __): error(e) {}
    symbol(enum symbol_type t): type(t) {} 
    symbol(string_literal literal): type(symbol_type::string_literal), string(literal) {}    
    symbol(llvm_literal literal): type(symbol_type::llvm_literal), llvm(literal) {}    
    symbol(struct identifier id): type(symbol_type::identifier), identifier(id) {}
    symbol(expression_list es): type(symbol_type::subexpression), expressions(es) {}
    symbol(expression e): symbol(expression_list {{e}}) {}
};

#endif /* nodes_hpp */
