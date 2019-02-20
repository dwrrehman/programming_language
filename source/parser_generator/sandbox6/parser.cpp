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

#include "color.h"

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

#define optional() level--; stack_trace.pop_back(); return begin(save, self);

#define operator_(op)     terminal(operator_type, op, p)
#define keyword_(kw)      terminal(keyword_type, kw, p)

#define b                 begin(save, self)

static bool begin(int save, node &self) {
    if (level > deepest_level) {
        deepest_level = level;
        deepest_node = self;
        deepest_stack_trace = stack_trace;
    }
    if (deepest_pointer < pointer)
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
    if (level > deepest_level) {
        deepest_level = level;
        deepest_node = parent;
        deepest_stack_trace = stack_trace;
    }
    if (deepest_pointer < pointer)
        deepest_pointer = pointer;
    
    return true;
}

static bool terminal(enum token_type desired_token_type, std::string desired_token_value, params) {
    if (level > deepest_level) {
        deepest_level = level;
        deepest_node = parent;
        deepest_stack_trace = stack_trace;
    }
    if (deepest_pointer < pointer)
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


//this is a automatically generated parser in cpp, for my language.


bool program(params);




//EBNF Parse Nodes:

bool program(params) {
	declare_node();
	if (b && keyword_("using") && identifier(p) && string(p) && number(p)) return success(parent, self);
	if (b && keyword_("using") && string(p) && identifier(p)) return success(parent, self);
	if (b && keyword_("for") && number(p) && operator_("{") && operator_("}")) return success(parent, self);
	return failure(save, self);
}



/// Hand made EBNF nodes:

bool identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", p)) return success(parent, self);
    return failure(save, self);
}

bool number(params){
    declare_node();
    if (begin(save, self) && terminal(number_type, "", p)) return success(parent, self);
    return failure(save, self);
}

bool string(params){
    declare_node();
    if (begin(save, self) && terminal(string_type, "", p)) return success(parent, self);
    return failure(save, self);
}

bool free_identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", p)) return success(parent, self);
    for (auto keyword : overridable_keywords) {
        if (begin(save, self) && terminal(keyword_type, keyword, p)) return success(parent, self);
        if (begin(save, self) && terminal(operator_type, keyword, p)) return success(parent, self);
    }
    return failure(save, self);
}

bool kind_free_identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", p)) return success(parent, self);
    for (auto keyword : overridable_keywords) {
        if (keyword == "[" || keyword == "]") continue;
        if (b && terminal(keyword_type, keyword, p)) return success(parent, self);
        if (b && terminal(operator_type, keyword, p)) return success(parent, self);
    }
    return failure(save, self);
}

bool qualifier(params) {
    declare_node();
    for (auto qualifier : qualifiers)
        if (b && keyword_(qualifier)) return success(parent, self);
    return failure(save, self);
}

bool required_newlines(params) {
    declare_node();
    if (begin(save, self) && operator_("\n") && required_newlines(p)) return success(parent, self);
    if (begin(save, self) && operator_("\n")) return success(parent, self);
    return failure(save, self);
}

bool newlines(params) {
    declare_node();
    if (begin(save, self) && operator_("\n") && newlines(p)) return success(parent, self);
    optional();
}

bool documentation(params) {
    declare_node();
    if (begin(save, self) && terminal(documentation_type, "", p) && newlines(p)) return success(parent, self);
    optional();
}


bool terminated_statement(params) {
    declare_node();
    if (b && statement(p) && tokens[pointer+1].type == operator_type && (tokens[pointer+1].value == "}")) return success(parent, self);
    if (b && statement(p) && required_newlines(p)) return success(parent, self);
    if (b && statement(p) && operator_(";")) return success(parent, self);
    return failure(save, self);
}

node parse(std::string text, std::vector<struct token> tokens, bool &error) {
    print_lex(tokens);
    node tree = {};
    bool parse_successful = false;
    
    if (!program(tokens, tree) || pointer != tokens.size()) {
        std::cout << "Error: parsing failed." << std::endl;
        
        int i = 0;
        for (auto n : deepest_stack_trace) {
            print_node(n, i++);
        }
        
    } else parse_successful = true;
    
    print_parse(tree);
    std::cout << "level = " << level << "\n";
    
    if (parse_successful && !level) {
        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "\n\n\n\t\tsuccessfully parsed.\n\n\n" << std::endl;
        std::cout << "------------------------------------------------" << std::endl;
    } else {
        std::cout << "\n\n\n\t\tPARSE FAILURE.\n\n\n\n" << std::endl;
        
        std::cout << "Expected a \"" << deepest_node.name << "\", Found a \"" << tokens[deepest_pointer - 1].value;
        std::cout << "\", of type: " << convert_token_type_representation(tokens[deepest_pointer - 1].type) << std::endl << std::endl;
        
        auto & t = tokens[deepest_pointer - 1];
        std::vector<int> offsets = {-2, -1, 0, 1, 2};
        std::string line = "";
        std::istringstream s {text};
        std::vector<std::string> lines = {};
        while (std::getline(s, line)) lines.push_back(line);
        
        for (auto offset : offsets) {
            
            size_t index = 0;
            if ((int) t.line - 1 + offset >= 0 && (int) t.line - 1 + offset < lines.size()) {
                index = t.line - 1 + offset;
            } else continue;
            
            std::cout << "\t" << GRAY << t.line + offset << RESET GREEN "  |  " RESET << lines[index] << std::endl;
            
            if (!offset) {
                std::cout << "\t";
                for (int i = 0; i < t.column + 5; i++) std::cout << " ";
                std::cout << BRIGHT_RED << "^";
                if (t.value.size() > 1) for (int i = 0; i < t.value.size() - 1; i++) std::cout << "~";
                std::cout << RESET << std::endl;
            }
        }
        
        std::cout << std::endl;
        error = true;
    }
    
    return tree;
}
