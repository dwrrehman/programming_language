//
//  parser.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <vector>

#include "parser.hpp"
#include "lexer.hpp"
#include "lists.hpp"




/**


one thing we want to be able to do:
 
 
 
        if condition {print "hi"}
 
 
 
 ;; notice that we ARENT required to put a new line, if there is a closing brace next.
 
 
 how will our parse do that? will it check the token[pointer] directly?
 


*/








#define prep(_level) for (int i = _level; i--;) std::cout << ".   "

void print_node(node &self, int level) {
    prep(level); std::cout << self.name << std::endl;
    if (self.data.type != null_type) {
        prep(level); std::cout << "type = " << convert_token_type_representation(self.data.type) << std::endl;
    }
    if (self.data.value != "") {
        prep(level); std::cout << "value = " << self.data.value << std::endl;
    }
    int i = 0;
    for (auto childnode : self.children) {
        std::cout << std::endl;
        if (self.children.size() > 1) {prep(level+1); std::cout << "child #" << i++ << ": " << std::endl;}
        print_node(childnode, level+1);
    }
}

void print_parse(node &tree) {
    std::cout << "------------ PARSE: ------------- " << std::endl;
    print_node(tree, 0);
}



/// Parsing:

int pointer = 0;

#define declare_node()    node self = node(__func__, {}, {}, true); int save = pointer;
#define params            std::vector<struct token> tokens, node &parent

static bool begin(int save, node &self) {
    pointer = save;
    self.children.clear();
    return true;
}
static bool failure(int save, node &self) {
    pointer = save;
    self.children.clear();
    return false;
}
static bool success(node &parent, const node &self) {
    parent.children.push_back(self);
    return true;
}
static bool push_terminal(node &parent, std::vector<struct token> &tokens) {
    parent.children.push_back(node("terminal", tokens[pointer++], {}, true));
    return true;
}

static bool terminal(enum token_type desired_token_type, std::string desired_token_value, params) {
    if (pointer >= tokens.size()) return false;
    if (desired_token_type == identifier_type && tokens[pointer].type == identifier_type) return push_terminal(parent, tokens);
    if (desired_token_type == string_type && tokens[pointer].type == string_type) return push_terminal(parent, tokens);
    if (desired_token_type == real_type && tokens[pointer].type == real_type) return push_terminal(parent, tokens);
    if (desired_token_type == integer_type && tokens[pointer].type == integer_type) return push_terminal(parent, tokens);
    if (desired_token_type == tokens[pointer].type && desired_token_value == tokens[pointer].value) return push_terminal(parent, tokens);
    return false;
}

static bool identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool identifier_or_keyword(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", tokens, self)) return success(parent, self);
    for (auto keyword : overridable_keywords) {
        if (begin(save, self) && terminal(keyword_type, keyword, tokens, self)) return success(parent, self);
    }
    return failure(save, self);
}

static bool symbol(params) {
    declare_node();
    if (begin(save, self) && terminal(integer_type, "", tokens, self)) return success(parent, self);
    if (begin(save, self) && terminal(real_type, "", tokens, self)) return success(parent, self);
    if (begin(save, self) && terminal(string_type, "", tokens, self)) return success(parent, self);
    if (begin(save, self) && identifier(tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool required_newlines(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, "\n", tokens, self) && required_newlines(tokens, self)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "\n", tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool newlines(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, "\n", tokens, self) && newlines(tokens, self)) return success(parent, self);
    return begin(save, self);
}

static bool type(params) {
    declare_node();
    for (auto type : builtin_types) {
        if (begin(save, self) && terminal(keyword_type, type, tokens, self)) return success(parent, self);
    }
    return failure(save, self);
}

// unimplemented
static bool expression(params) {
    declare_node();
    if (begin(save, self) && terminal(real_type, "", tokens, self)) return success(parent, self); // temp.
    return failure(save, self);
}

static bool identifier_list(params) {
    declare_node();
    if (begin(save, self) && identifier_or_keyword(tokens, parent) && identifier_list(tokens, self)) return success(parent, self);
    return begin(save, self);
}

static bool parameter(params) {
    declare_node();
    if (begin(save, self) && identifier_or_keyword(tokens, self) && terminal(operator_type, ":", tokens, self) && type(tokens, self) && terminal(operator_type, "=", tokens, self) && expression(tokens, self)) return success(parent, self);
    if (begin(save, self) && identifier_or_keyword(tokens, self) && terminal(operator_type, ":", tokens, self) && type(tokens, self)) return success(parent, self);
    if (begin(save, self) && identifier_or_keyword(tokens, self) && terminal(operator_type, "=", tokens, self) && expression(tokens, self)) return success(parent, self);
    if (begin(save, self) && identifier_or_keyword(tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool parameter_list(params) {
    declare_node();
    if (begin(save, self) && parameter(tokens, self) && parameter_list(tokens, self)) return success(parent, self);
    return begin(save, self);
}

static bool optional_signature(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, "(", tokens, self) && parameter_list(tokens, self) && terminal(operator_type, ")", tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool optional_signature_list(params) {
    declare_node();
    if (begin(save, self) && optional_signature(tokens, self) && optional_signature_list(tokens, self)) return success(parent, self);
    return begin(save, self);
}

static bool parameter_signature(params) {
    declare_node();
    if (begin(save, self) && parameter_list(tokens, self) && optional_signature_list(tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool qualifier(params) {
    declare_node();
    for (auto qualifier : qualifiers) {
        if (begin(save, self) && terminal(keyword_type, qualifier, tokens, self)) return success(parent, self);
    }
    return failure(save, self);
}

static bool qualifier_list(params) {
    declare_node();
    if (begin(save, self) && qualifier(tokens, self) && qualifier_list(tokens, self)) return success(parent, self);
    return begin(save, self);
}

static bool capture_list(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, "[", tokens, self) && identifier_list(tokens, self) && terminal(operator_type, "]", tokens, self)) return success(parent, self);
    return begin(save, self);
}

static bool statement_list(params);

static bool if_head_statement(params) {
    declare_node();
    if (begin(save, self) && terminal(keyword_type, "if", tokens, self) && expression(tokens, self) && terminal(operator_type, "{", tokens, self) && statement_list(tokens, self) && terminal(operator_type, "}", tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool else_if_statement(params) {
    declare_node();
    if (begin(save, self) && terminal(keyword_type, "else", tokens, self) && terminal(keyword_type, "if", tokens, self) && expression(tokens, self) && terminal(operator_type, "{", tokens, self) && statement_list(tokens, self) && terminal(operator_type, "}", tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool else_if_statement_list(params) {
    declare_node();
    if (begin(save, self) && else_if_statement(tokens, self) && else_if_statement_list(tokens, self)) return success(parent, self);
    return begin(save, self);
}

static bool else_statement(params) {
    declare_node();
    if (begin(save, self) && terminal(keyword_type, "else", tokens, self) && terminal(operator_type, "{", tokens, self) && statement_list(tokens, self) && terminal(operator_type, "}", tokens, self)) return success(parent, self);
    return begin(save, self);
}

static bool if_statement(params) {
    declare_node();
    if (begin(save, self) && if_head_statement(tokens, self) && newlines(tokens, self) && else_if_statement_list(tokens, self) && newlines(tokens, self) && else_statement(tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool assignment_statement(params) {
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", tokens, self) && terminal(operator_type, "=", tokens, self) && expression(tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool statement(params) {
    declare_node();
    if (begin(save, self) && assignment_statement(tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool unterminated_statement(params) {
    declare_node();
    if (begin(save, self) && if_statement(tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool terminated_statement(params) {
    declare_node();
    if (begin(save, self) && statement(tokens, self) && required_newlines(tokens, self)) return success(parent, self);
    if (begin(save, self) && statement(tokens, self) && tokens[pointer+1].type == operator_type && tokens[pointer+1].value == "}") return success(parent, self);
    return failure(save, self);
}

static bool statement_list(params) {
    declare_node();
    if (begin(save, self) && newlines(tokens, self) && terminated_statement(tokens, self) && statement_list(tokens, self)) return success(parent, self);
    if (begin(save, self) && newlines(tokens, self) && unterminated_statement(tokens, self) && statement_list(tokens, self)) return success(parent, self);
    if (begin(save, self) && newlines(tokens, self)) return success(parent, self);
    return begin(save, self);
}

static bool function_declaration(params) {
    declare_node();
    if (begin(save, self) && qualifier_list(tokens, self) && terminal(operator_type, "(", tokens, self) && parameter_signature(tokens, self) && terminal(operator_type, ")", tokens, self) && capture_list(tokens, self) && terminal(operator_type, "{", tokens, self) && statement_list(tokens, self) && terminal(operator_type, "}", tokens, self) && required_newlines(tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool declaration(params) {
    declare_node();
    if (begin(save, self) && function_declaration(tokens, self)) return success(parent, self);
    return failure(save, self);
}

static bool declaration_list(params) {
    declare_node();
    if (begin(save, self) && declaration(tokens, self) && declaration_list(tokens, self)) return success(parent, self);
    return begin(save, self);
}

static bool program(params) {
    declare_node();
    if (begin(save, self) && declaration_list(tokens, self)) return success(parent, self);
    return failure(save, self);
}

node parse(std::vector<struct token> tokens) {
    print_lex(tokens);
    node tree = {};
    
    if (!program(tokens, tree) || pointer != tokens.size())
        std::cout << "Error: parsing failed." << std::endl;
        
    else
        std::cout << "successfully parsed." << std::endl;
    
    
    print_parse(tree);
    
    return tree;
}
