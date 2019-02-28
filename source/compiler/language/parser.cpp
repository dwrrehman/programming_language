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

bool skipped = false;
std::string the_filename = "";
std::string the_text = "";

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



//this is a automatically generated parser in cpp, for my language.


bool program(params);

bool translation_unit(params);

bool declaration_list(params);

bool declaration(params);

bool implementation_declaration(params);

bool variable_implementation_declaration(params);

bool assignment_statement(params);

bool expression(params);

bool type_expression(params);

bool free_identifier_or_symbol_list(params);

bool symbol(params);

bool expression_list(params);

bool import_statement(params);

bool using_statement(params);

bool used_list(params);

bool used_element(params);

bool module_list(params);

bool module(params);

bool filename_list(params);

bool filename(params);




//EBNF Parse Nodes:

bool program(params) {
    declare_node();
    if (b && translation_unit(p)) return success(parent, self);
    return failure(save, self);
}

bool translation_unit(params) {
    declare_node();
    if (b && declaration_list(p)) return success(parent, self);
    return failure(save, self);
}

bool declaration_list(params) {
    declare_node();
    if (b && newlines(p) && declaration(p) && required_newlines(p) && declaration_list(p)) return success(parent, self);
    if (b && newlines(p)) return success(parent, self);
    optional();
}

bool declaration(params) {
    declare_node();
    //deepest_level = 0;
    //deepest_pointer = 0;
    if (b && implementation_declaration(p)) return success(parent, self);
    
    //if (tokens[deepest_pointer].value != "\n") {
//        print_parse_error(the_filename, tokens[deepest_pointer - 1].line, tokens[deepest_pointer - 1].column, tokens[deepest_pointer - 1].type, tokens[deepest_pointer - 1].value);
//        print_source_code(the_text, {tokens[deepest_pointer - 1]});
//    }
    
    //while (tokens[pointer].value != "\n" && pointer < tokens.size()) {skipped = true; pointer++;}
    
    return failure(save, self);
}

bool implementation_declaration(params) {
    declare_node();
    if (b && variable_implementation_declaration(p)) return success(parent, self);
    if (b && using_statement(p)) return success(parent, self);
    if (b && import_statement(p)) return success(parent, self);
    return failure(save, self);
}

bool variable_implementation_declaration(params) {
    declare_node();
    if (b && assignment_statement(p)) return success(parent, self);
    return failure(save, self);
}

bool assignment_statement(params) {
    declare_node();
    if (b && identifier(p) && operator_(":") && type_expression(p) && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("+") && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("-") && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("*") && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("/") && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("%") && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("|") && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("&") && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("^") && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("!") && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("~") && operator_("=") && expression(p)) return success(parent, self);
    return failure(save, self);
}

bool expression(params) {
    declare_node();
    if (b && free_identifier_or_symbol_list(p)) return success(parent, self);
    return failure(save, self);
}

bool type_expression(params) {
    declare_node();
    if (b && operator_("(") && expression(p) && operator_(")")) return success(parent, self);
    if (b && operator_("{") && expression(p) && operator_("}")) return success(parent, self);
    if (b && identifier(p)) return success(parent, self);
    if (b && free_identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool free_identifier_or_symbol_list(params) {
    declare_node();
    if (b && symbol(p) && free_identifier_or_symbol_list(p)) return success(parent, self);
    if (b && free_identifier(p) && free_identifier_or_symbol_list(p)) return success(parent, self);
    optional();
}

bool symbol(params) {
    declare_node();
    if (b && operator_("(") && expression_list(p) && operator_(")")) return success(parent, self);
    if (b && operator_("{") && expression_list(p) && operator_("}")) return success(parent, self);
    if (b && number(p)) return success(parent, self);
    if (b && string(p)) return success(parent, self);
    if (b && identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool expression_list(params) {
    declare_node();
    if (b && expression(p) && operator_(",") && expression_list(p)) return success(parent, self);
    if (b && expression(p) && operator_(",")) return success(parent, self);
    if (b && expression(p)) return success(parent, self);
    optional();
}

bool import_statement(params) {
    declare_node();
    if (b && keyword_("import") && module_list(p)) return success(parent, self);
    return failure(save, self);
}

bool using_statement(params) {
    declare_node();
    if (b && keyword_("using") && used_list(p) && keyword_("from") && module(p)) return success(parent, self);
    if (b && keyword_("using") && module_list(p)) return success(parent, self);
    return failure(save, self);
}

bool used_list(params) {
    declare_node();
    if (b && used_element(p) && operator_(",") && used_list(p) && used_list(p)) return success(parent, self);
    if (b && used_element(p)) return success(parent, self);
    return failure(save, self);
}

bool used_element(params) {
    declare_node();
    if (b && identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool module_list(params) {
    declare_node();
    if (b && module(p) && operator_(",") && module_list(p)) return success(parent, self);
    if (b && module(p)) return success(parent, self);
    return failure(save, self);
}

bool module(params) {
    declare_node();
    if (b && identifier(p) && filename_list(p)) return success(parent, self);
    if (b && identifier(p)) return success(parent, self);
    if (b && filename_list(p)) return success(parent, self);
    return failure(save, self);
}

bool filename_list(params) {
    declare_node();
    if (b && filename(p) && filename_list(p)) return success(parent, self);
    if (b && filename(p)) return success(parent, self);
    return failure(save, self);
}

bool filename(params) {
    declare_node();
    if (b && operator_(".") && identifier(p)) return success(parent, self);
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
    //if (b && statement(p) && tokens[pointer+1].type == operator_type && (tokens[pointer+1].value == "}")) return success(parent, self);
    //if (b && statement(p) && required_newlines(p)) return success(parent, self);
    //if (b && statement(p) && operator_(";")) return success(parent, self);
    return failure(save, self);
}

node parse(std::string filename, std::string text, std::vector<struct token> tokens, bool &error) {
    
    print_lex(tokens);
    node tree = {};
    the_filename = filename;
    the_text = text;
    if (!program(tokens, tree) || pointer != tokens.size() || level || skipped) {
        
        std::cout << "level = " << level << std::endl;
        std::cout << "pointer = " << pointer << ", tokens.size() = " << tokens.size() << std::endl;
        
        int i = 0;
        for (auto n : deepest_stack_trace) print_node(n, i++);
        print_parse(tree);
        print_parse_error(filename, tokens[deepest_pointer - 1].line, tokens[deepest_pointer - 1].column, tokens[deepest_pointer - 1].type, tokens[deepest_pointer - 1].value);
        print_source_code(text, {tokens[deepest_pointer - 1]});
        error = true;
        
    } else {
        print_parse(tree);
        std::cout << "\n\n\tsuccess.\n\n" << std::endl;
    }
    
    return tree;
}
