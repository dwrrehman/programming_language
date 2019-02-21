//
//  parser.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <sstream>
#include <iostream>
#include <vector>

#include "parser.hpp"
#include "lexer.hpp"
#include "lists.hpp"
#include "error.hpp"

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


/// Parsing:

static int pointer = 0;
static std::vector<node> stack_trace = {};

static int deepest_pointer = 0;
static node deepest_node = {};
static std::vector<node> deepest_stack_trace = {};

size_t deepest_level = 0;
size_t level = 0;

#define declare_node()    node self = node(__func__, {}, {}, true);                       \
int save = pointer;                                             \
stack_trace.push_back(self);                                    \
level++;                                                        \

#define params            std::vector<struct token> tokens, node &parent
#define p                 tokens, self

#define optional()        level--; stack_trace.pop_back(); return begin(save, self);

#define operator_(op)     terminal(operator_type, op, p)
#define keyword_(kw)      terminal(keyword_type, kw, p)

#define b                 begin(save, self)

static bool begin(int save, node &self) {
    if (level >= deepest_level) {
        deepest_level = level;
        deepest_node = self;
        deepest_stack_trace = stack_trace;
    }
    if (deepest_pointer <= pointer)
        deepest_pointer = pointer;
    
    pointer = save;
    self.children.clear();
    return true;
}
static bool failure(int save, node &self) {
    pointer = save;
    self.children.clear();
    level--;
    stack_trace.pop_back();
    return false;
}
static bool success(node &parent, const node &self) {
    parent.children.push_back(self);
    level--;
    stack_trace.pop_back();
    return true;
}
static bool push_terminal(node &parent, std::vector<struct token> &tokens) {
    parent.children.push_back(node("terminal", tokens[pointer++], {}, true));
    if (level >= deepest_level) {
        deepest_level = level;
        deepest_node = parent;
        deepest_stack_trace = stack_trace;
    }
    if (deepest_pointer <= pointer)
        deepest_pointer = pointer;
    
    return true;
}

static bool terminal(enum token_type desired_token_type, std::string desired_token_value, params) {
    if (level >= deepest_level) {
        deepest_level = level;
        deepest_node = parent;
        deepest_stack_trace = stack_trace;
    }
    if (deepest_pointer <= pointer)
        deepest_pointer = pointer;
    
    if (pointer >= tokens.size()) return false;
    if (desired_token_type == identifier_type && tokens[pointer].type == identifier_type) return push_terminal(parent, tokens);
    if (desired_token_type == string_type && tokens[pointer].type == string_type) return push_terminal(parent, tokens);
    if (desired_token_type == documentation_type && tokens[pointer].type == documentation_type) return push_terminal(parent, tokens);
    if (desired_token_type == number_type && tokens[pointer].type == number_type) return push_terminal(parent, tokens);
    if (desired_token_type == tokens[pointer].type && desired_token_value == tokens[pointer].value) return push_terminal(parent, tokens);
    return false;
}


/// Hand made EBNF nodes interfaces:

bool identifier(params);

bool number(params);

bool string(params);

bool free_identifier(params);

bool type_free_identifier(params);

bool kind_free_identifier(params);

bool qualifier(params);

bool required_newlines(params);

bool newlines(params);

bool documentation(params);

bool terminated_statement(params);

