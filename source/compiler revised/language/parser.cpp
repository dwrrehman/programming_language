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

#define prep(_level) for (int i = _level; i--;) std::cout << ".   "

void print_node(node &self, int level) {
    prep(level); std::cout << self.name << " (" << self.children.size() << ")" << std::endl;
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
bool space_declaration(params);

bool generic_parameter_declaration_list(params);
bool parameter_declaration_list(params);
bool statement_list(params);
bool expression(params);
bool expression_list(params);
bool type_expression(params);
bool terminated_statement(params);
bool statement(params);
bool declaration_list(params);










/// Main Parser Code:

static bool identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", p)) return success(parent, self);
    return failure(save, self);
}

static bool free_identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", p)) return success(parent, self);
    for (auto keyword : overridable_keywords) {
        if (begin(save, self) && terminal(keyword_type, keyword, p)) return success(parent, self);
        if (begin(save, self) && terminal(operator_type, keyword, p)) return success(parent, self);
    }
    return failure(save, self);
}

static bool generic_free_identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", p)) return success(parent, self);
    for (auto keyword : overridable_keywords) {
        if (keyword == "[" || keyword == "]") continue;
        if (b && terminal(keyword_type, keyword, p)) return success(parent, self);
        if (b && terminal(operator_type, keyword, p)) return success(parent, self);
    }
    return failure(save, self);
}

static bool symbol(params) {
    declare_node();
    if (begin(save, self) && operator_("(") && expression_list(p) && operator_(")")) return success(parent, self);
    if (begin(save, self) && operator_("{") && expression_list(p) && operator_("}")) return success(parent, self);
    if (begin(save, self) && operator_("[") && expression_list(p) && operator_("]")) return success(parent, self);
    if (begin(save, self) && terminal(number_type, "", p)) return success(parent, self);
    if (begin(save, self) && terminal(string_type, "", p)) return success(parent, self);
    if (begin(save, self) && identifier(p)) return success(parent, self);
    return failure(save, self);
}

static bool required_newlines(params) {
    declare_node();
    if (begin(save, self) && operator_("\n") && required_newlines(p)) return success(parent, self);
    if (begin(save, self) && operator_("\n")) return success(parent, self);
    return failure(save, self);
}

static bool newlines(params) {
    declare_node();
    if (begin(save, self) && operator_("\n") && newlines(p)) return success(parent, self);
    optional();
}

static bool documentation(params) {
    declare_node();
    if (begin(save, self) && terminal(documentation_type, "", p) && newlines(p)) return success(parent, self);
    optional();
}

static bool type_expression_list(params) {
    declare_node();
    if (begin(save, self) && type_expression(p) && operator_(",") && type_expression_list(p)) return success(parent, self);
    if (begin(save, self) && free_identifier(p) && operator_(",") && type_expression_list(p)) return success(parent, self);
    if (begin(save, self) && type_expression(p) && type_expression_list(p)) return success(parent, self);
    if (begin(save, self) && free_identifier(p) && type_expression_list(p)) return success(parent, self);
    optional();
}

static bool free_identifier_or_symbol_list(params) {
    declare_node();
    if (begin(save, self) && free_identifier(p) && free_identifier_or_symbol_list(p)) return success(parent, self);
    if (begin(save, self) && symbol(p) && free_identifier_or_symbol_list(p)) return success(parent, self);
    optional();
}

bool type_expression(params) {
    declare_node();
    if (begin(save, self) && operator_("[") && type_expression_list(p) && operator_("]")) return success(parent, self);
    if (begin(save, self) && operator_("{") && type_expression_list(p) && operator_("}")) return success(parent, self);
    if (begin(save, self) && operator_("(") && type_expression_list(p) && operator_(")")) return success(parent, self);
    if (begin(save, self) && free_identifier_or_symbol_list(p)) return success(parent, self);
    return failure(save, self);
}

bool expression(params) {
    declare_node();
    if (begin(save, self) && free_identifier_or_symbol_list(p)) return success(parent, self);
    return failure(save, self);
}

bool expression_list(params) {
    declare_node();
    if (begin(save, self) && expression(p) && expression_list(p)) return success(parent, self);
    optional();
}

static bool open_block(params) {
    declare_node();
    
    return failure(save, self);
}

static bool block(params) {
    declare_node();
    if (begin(save, self) && operator_("{") && statement_list(p) && operator_("}")) return success(parent, self);
    return failure(save, self);
}

static bool open_declaration_block(params) {
    declare_node();
    if (begin(save, self) && operator_("{") && declaration_list(p) && operator_("}")) return success(parent, self);
    return failure(save, self);
}

static bool closed_declaration_block(params) {
    declare_node();
    if (begin(save, self) && operator_("{") && declaration_list(p) && operator_("}")) return success(parent, self);
    return failure(save, self);
}

static bool declaration_block(params) {
    declare_node();
    if (begin(save, self) && open_declaration_block(p)) return success(parent, self);
    if (begin(save, self) && closed_declaration_block(p)) return success(parent, self);
    return failure(save, self);
}

static bool if_head_statement(params) {
    declare_node();
    if (begin(save, self) && keyword_("if") && expression(p) && block(p)) return success(parent, self);
    if (begin(save, self) && keyword_("if") && expression(p) && operator_(",") && newlines(p) && statement(p)) return success(parent, self);
    return failure(save, self);
}

static bool else_if_statement(params) {
    declare_node();
    if (b && keyword_("else") && if_head_statement(p)) return success(parent, self);
    return failure(save, self);
}

static bool else_if_statement_list(params) {
    declare_node();
    if (b && newlines(p) && else_if_statement(p) && else_if_statement_list(p)) return success(parent, self);
    optional();
}

static bool else_statement(params) {
    declare_node();
    if (b && newlines(p) && keyword_("else") && block(p)) return success(parent, self);
    if (b && newlines(p) && keyword_("else") && newlines(p) && statement(p)) return success(parent, self);
    optional();
}

static bool if_statement(params) {
    declare_node();
    if (b && if_head_statement(p) && else_if_statement_list(p) && else_statement(p)) return success(parent, self);
    return failure(save, self);
}

static bool while_statement(params) {
    declare_node();
    if (b && keyword_("while") && expression(p) && block(p)) return success(parent, self);
    if (b && keyword_("while") && expression(p) && operator_(",") && newlines(p) && statement(p)) return success(parent, self);
    return failure(save, self);
}

static bool repeat_while_statement(params) {
    declare_node();
    if (begin(save, self) && keyword_("repeat") && block(p) && keyword_("while") && expression(p)) return success(parent, self);
    return failure(save, self);
}

static bool for_statement(params) {
    declare_node();
    if (b && keyword_("for") && identifier(p) && keyword_("in") && expression(p) && block(p)) return success(parent, self);
    if (b && keyword_("for") && identifier(p) && keyword_("in") && expression(p) && operator_(",") && newlines(p) && statement(p)) return success(parent, self);
    return failure(save, self);
}

static bool return_statement(params) {
    declare_node();
    if (b && keyword_("return") && expression(p)) return success(parent, self);
    if (b && keyword_("return")) return success(parent, self);
    return failure(save, self);
}

static bool assignment_declaration_statement(params) {
    declare_node();
    if (b && terminal(identifier_type, "", p) && operator_(":") && type_expression(p) && operator_("=") && expression(p)) return success(parent, self);
    if (b && terminal(identifier_type, "", p) && operator_("=") && expression(p)) return success(parent, self);
    return failure(save, self);
}

static bool assignment_statement(params) {
    declare_node();
    if (b && terminal(identifier_type, "", p) && operator_("=") && expression(p)) return success(parent, self);
    return failure(save, self);
}

bool statement(params) {
    declare_node();
    if (b && if_statement(p)) return success(parent, self);
    if (b && while_statement(p)) return success(parent, self);
    if (b && repeat_while_statement(p)) return success(parent, self);
    if (b && return_statement(p)) return success(parent, self);
    if (b && for_statement(p)) return success(parent, self);
    if (b && assignment_declaration_statement(p)) return success(parent, self);
    if (b && assignment_statement(p)) return success(parent, self);
    if (b && block(p)) return success(parent, self);
    if (b && expression(p)) return success(parent, self);
    return failure(save, self);
}

bool terminated_statement(params) {
    declare_node();
    if (b && statement(p) && tokens[pointer+1].type == operator_type && (tokens[pointer+1].value == "}")) return success(parent, self);
    if (b && statement(p) && required_newlines(p)) return success(parent, self);
    if (b && statement(p) && operator_(";")) return success(parent, self);
    return failure(save, self);
}

bool statement_list(params) {
    declare_node();
    if (b && newlines(p) && terminated_statement(p) && statement_list(p)) return success(parent, self);
    if (b && newlines(p)) return success(parent, self);
    optional();
}

static bool inheritance_declaration(params) {
    declare_node();
    if (b && operator_(":") && type_expression(p)) return success(parent, self);
    optional();
}

static bool type_signature(params) {
    declare_node();
    if (b && operator_("{") && parameter_declaration_list(p) && operator_("}")) return success(parent, self);
    return failure(save, self);
}

static bool generic_type_signature(params) {
    declare_node();
    if (b && operator_("[") && generic_parameter_declaration_list(p) && operator_("]")) return success(parent, self);
    optional();
}

static bool type_delcaration(params) {
    declare_node();
    if (b && generic_type_signature(p) && type_signature(p) && inheritance_declaration(p) && block(p)) return success(parent, self);
    return failure(save, self);
}

static bool type_interface_declaration(params) {
    declare_node();
    if (b && documentation(p) && type_delcaration(p)) return success(parent, self);
    return failure(save, self);
}

bool space_declaration(params) {
    declare_node();
    if (b && identifier(p) && declaration_block(p)) return success(parent, self);
    return failure(save, self);
}

static bool space_interface_declaration(params) {
    declare_node();
    if (b && documentation(p) && space_declaration(p)) return success(parent, self);
    return failure(save, self);
}

static bool variable_interface_declaration(params) {
    declare_node();
    if (b && documentation(p) && identifier(p) && operator_(":") && type_expression(p)) return success(parent, self);
    return failure(save, self);
}

static bool parameter_declaration(params) {
    declare_node();
    if (b && identifier(p) && operator_(":") && type_expression(p) && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_(":") && type_expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("=") && expression(p)) return success(parent, self);
    if (b && free_identifier(p)) return success(parent, self);
    return failure(save, self);
}

static bool generic_parameter_declaration(params) {
    declare_node();
    if (b && identifier(p) && operator_(":") && type_expression(p) &&  operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_(":") && type_expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("=") && expression(p)) return success(parent, self);
    if (b && generic_free_identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool parameter_declaration_list(params) {
    declare_node();
    if (b && parameter_declaration(p) && parameter_declaration_list(p)) return success(parent, self);
    optional();
}

bool generic_parameter_declaration_list(params) {
    declare_node();
    if (b && generic_parameter_declaration(p) && generic_parameter_declaration_list(p)) return success(parent, self);
    optional();
}

static bool optional_function_signature(params) {
    declare_node();
    if (b && operator_("(") && parameter_declaration_list(p) &&  operator_(")")) return success(parent, self);
    return failure(save, self);
}

static bool optional_function_signature_list(params) {
    declare_node();
    if (b && optional_function_signature(p) && optional_function_signature_list(p)) return success(parent, self);
    optional();
}

static bool function_call_signature(params) {
    declare_node();
    if (b && operator_("(") && parameter_declaration_list(p) && optional_function_signature_list(p) &&  operator_(")")) return success(parent, self);
    return failure(save, self);
}

static bool return_type_signature(params) {
    declare_node();
    if (b &&  operator_("-") &&  operator_(">") && type_expression(p)) return success(parent, self);
    optional();
}

static bool qualifier(params) {
    declare_node();
    for (auto qualifier : qualifiers)
        if (b && keyword_(qualifier)) return success(parent, self);
    return failure(save, self);
}

static bool qualifier_list(params) {
    declare_node();
    if (b && qualifier(p) && qualifier_list(p)) return success(parent, self);
    optional();
}

static bool precedence_signature(params) {
    declare_node();
    if (b && operator_("<") && function_call_signature(p) && operator_("+") && terminal(number_type, "", p) && operator_(">")) return success(parent, self);
    if (b && operator_("<") && terminal(number_type, "", p) && operator_(">")) return success(parent, self);
    optional();
}

static bool function_signature(params) {
    declare_node();
    if (b && function_call_signature(p) && return_type_signature(p) && generic_type_signature(p) && precedence_signature(p) && qualifier_list(p)) return success(parent, self);
    return failure(save, self);
}

static bool function_interface_declaration(params) {
    declare_node();
    if (b && documentation(p) && function_signature(p)) return success(parent, self);
    return failure(save, self);
}

static bool function_implementation_declaration(params) {
    declare_node();
    if (b && function_signature(p) && block(p)) return success(parent, self);
    return failure(save, self);
}

static bool variable_implementation_declaration(params) {
    declare_node();
    if (b && assignment_declaration_statement(p)) return success(parent, self);
    return failure(save, self);
}

static bool implementation_declaration(params) {
    declare_node();
    if (b && function_implementation_declaration(p)) return success(parent, self);
    if (b && variable_implementation_declaration(p)) return success(parent, self);
    return failure(save, self);
}

static bool interface_declaration(params) {
    declare_node();
    if (b && function_interface_declaration(p)) return success(parent, self);
    if (b && type_interface_declaration(p)) return success(parent, self);
    if (b && space_interface_declaration(p)) return success(parent, self);
    if (b && variable_interface_declaration(p)) return success(parent, self);
    return failure(save, self);
}

static bool declaration(params) {
    declare_node();
    deepest_level = 0;
    if (b && implementation_declaration(p)) return success(parent, self);
    if (b && interface_declaration(p)) return success(parent, self);
    return failure(save, self);
}

bool declaration_list(params) {
    declare_node();
    if (b && newlines(p) && declaration(p) && required_newlines(p) && declaration_list(p)) return success(parent, self);
    if (b && newlines(p)) return success(parent, self);
    optional();
}

static bool program(params) {
    declare_node();
    if (b && declaration_list(p)) return success(parent, self);
    if (b && newlines(p)) return success(parent, self);
    return failure(save, self);
}

void print_token(struct token t) {
    std::cout << "Error at token: \n\n";
    std::cout << "\t\t---------------------------------\n";
    std::cout << "\t\tline " << t.line << "," << t.column << " : "<< t.value << "           "  <<  "(" << convert_token_type_representation(t.type) << ")\n";
    std::cout << "\t\t---------------------------------\n\n\n";
}

node parse(std::vector<struct token> tokens, bool &error) {
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
        
        std::cout << "current pointer:" << std::endl;
        print_token(tokens[pointer]);
        
        std::cout << "Expected a \"" << deepest_node.name << "\", Found a \"" << tokens[deepest_pointer - 1].value;
        std::cout << "\", of type: " << convert_token_type_representation(tokens[deepest_pointer - 1].type) << std::endl;
        
        std::cout << "\ndeepest pointer: (raw)" << std::endl;
        if (deepest_pointer - 1 >= tokens.size()) {
            std::cout << "...couldnt print unexpected found token. " << std::endl;
            std::cout << "deepest pointer = " << deepest_pointer - 1<< std::endl;
            std::cout << "token count = " << tokens.size() << std::endl;
        } else print_token(tokens[deepest_pointer - 1]);
        
        std::cout << "\n\n\n\n";
        std::cout << "current level: " << level << "\n";
        std::cout << "last error: on level " << deepest_level << "\n";
        print_node(deepest_node, 0);
        
        error = true;
    }
    
    return tree;
}