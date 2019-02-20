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

bool translation_unit(params);

bool declaration_list(params);

bool interface_declaration_list(params);

bool implementation_declaration_list(params);

bool interface_declaration_block(params);

bool implementation_declaration_block(params);

bool enum_interface_declaration_block(params);

bool enum_identifier_list(params);

bool enum_implementation_declaration_block(params);

bool enum_assignment_list(params);

bool enum_assignment(params);

bool declaration(params);

bool interface_declaration(params);

bool implementation_declaration(params);

bool function_interface_declaration(params);

bool variable_interface_declaration(params);

bool type_interface_declaration(params);

bool kind_interface_declaration(params);

bool space_interface_declaration(params);

bool enum_interface_declaration(params);

bool using_type_assignment_statement(params);

bool import_statement(params);

bool using_statement(params);

bool used_list(params);

bool used_element(params);

bool module_list(params);

bool module(params);

bool filename_list(params);

bool filename(params);

bool function_implementation_declaration(params);

bool variable_implementation_declaration(params);

bool type_implementation_declaration(params);

bool kind_implementation_declaration(params);

bool space_implementation_declaration(params);

bool enum_implementation_declaration(params);

bool function_signature(params);

bool type_signature(params);

bool kind_signature(params);

bool function_call_signature(params);

bool type_call_signature(params);

bool kind_call_signature(params);

bool parent_type(params);

bool precedence(params);

bool qualifier_list(params);

bool return_type(params);

bool parameter_list(params);

bool parameter(params);

bool optional_parameter_list(params);

bool kind_parameter_list(params);

bool kind_parameter(params);

bool required_kind_parameter(params);

bool optional_kind_parameter(params);

bool optional_kind_parameter_list(params);

bool required_parameter(params);

bool optional_parameter(params);

bool implementation_declaration_or_statement_list(params);

bool block(params);

bool code(params);

bool statement_list(params);

bool statement(params);

bool assignment_statement(params);

bool type_assignment_statement(params);

bool return_statement(params);

bool identifier_list(params);

bool for_statement(params);

bool repeatwhile_statement(params);

bool while_statement(params);

bool if_head_statement(params);

bool if_statement(params);

bool else_statement(params);

bool else_if_statement_list(params);

bool else_if_statement(params);

bool expression(params);

bool type_expression(params);

bool free_identifier_or_symbol_list(params);

bool symbol(params);

bool expression_list(params);




//EBNF Parse Nodes:

bool program(params) {
    declare_node();
    if (b && translation_unit(p)) return success(parent, self);
    return failure(save, self);
}

bool translation_unit(params) {
    declare_node();
    if (b && declaration_list(p)) return success(parent, self);
    if (b && newlines(p)) return success(parent, self);
    return failure(save, self);
}

bool declaration_list(params) {
    declare_node();
    if (b && newlines(p) && declaration(p) && required_newlines(p) && declaration_list(p)) return success(parent, self);
    optional();
}

bool interface_declaration_list(params) {
    declare_node();
    if (b && newlines(p) && interface_declaration(p) && required_newlines(p) && interface_declaration_list(p)) return success(parent, self);
    optional();
}

bool implementation_declaration_list(params) {
    declare_node();
    if (b && newlines(p) && implementation_declaration(p) && required_newlines(p) && implementation_declaration_list(p)) return success(parent, self);
    optional();
}

bool interface_declaration_block(params) {
    declare_node();
    if (b && operator_("{") && interface_declaration_list(p) && operator_("}")) return success(parent, self);
    return failure(save, self);
}

bool implementation_declaration_block(params) {
    declare_node();
    if (b && operator_("{") && implementation_declaration_list(p) && operator_("}")) return success(parent, self);
    return failure(save, self);
}

bool enum_interface_declaration_block(params) {
    declare_node();
    if (b && operator_("{") && enum_identifier_list(p) && operator_("}")) return success(parent, self);
    return failure(save, self);
}

bool enum_identifier_list(params) {
    declare_node();
    if (b && newlines(p) && identifier(p) && required_newlines(p) && enum_identifier_list(p)) return success(parent, self);
    if (b && newlines(p) && identifier(p) && operator_(",") && newlines(p) && enum_identifier_list(p)) return success(parent, self);
    optional();
}

bool enum_implementation_declaration_block(params) {
    declare_node();
    if (b && operator_("{") && enum_assignment_list(p) && operator_("}")) return success(parent, self);
    return failure(save, self);
}

bool enum_assignment_list(params) {
    declare_node();
    if (b && newlines(p) && enum_assignment(p) && required_newlines(p) && enum_assignment_list(p)) return success(parent, self);
    if (b && newlines(p) && enum_assignment(p) && operator_(",") && newlines(p) && enum_assignment_list(p)) return success(parent, self);
    optional();
}

bool enum_assignment(params) {
    declare_node();
    if (b && identifier(p) && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool declaration(params) {
    declare_node();
    deepest_level = 0;
    if (b && implementation_declaration(p)) return success(parent, self);
    if (b && interface_declaration(p)) return success(parent, self);
    return failure(save, self);
}

bool interface_declaration(params) {
    declare_node();
    if (b && function_interface_declaration(p)) return success(parent, self);
    if (b && type_interface_declaration(p)) return success(parent, self);
    if (b && space_interface_declaration(p)) return success(parent, self);
    if (b && enum_interface_declaration(p)) return success(parent, self);
    if (b && variable_interface_declaration(p)) return success(parent, self);
    if (b && kind_interface_declaration(p)) return success(parent, self);
    if (b && using_type_assignment_statement(p)) return success(parent, self);
    if (b && using_statement(p)) return success(parent, self);
    if (b && import_statement(p)) return success(parent, self);
    return failure(save, self);
}

bool implementation_declaration(params) {
    declare_node();
    if (b && function_implementation_declaration(p)) return success(parent, self);
    if (b && type_implementation_declaration(p)) return success(parent, self);
    if (b && space_implementation_declaration(p)) return success(parent, self);
    if (b && enum_implementation_declaration(p)) return success(parent, self);
    if (b && variable_implementation_declaration(p)) return success(parent, self);
    if (b && kind_implementation_declaration(p)) return success(parent, self);
    if (b && type_assignment_statement(p)) return success(parent, self);
    if (b && using_statement(p)) return success(parent, self);
    if (b && import_statement(p)) return success(parent, self);
    return failure(save, self);
}

bool function_interface_declaration(params) {
    declare_node();
    if (b && documentation(p) && function_signature(p)) return success(parent, self);
    return failure(save, self);
}

bool variable_interface_declaration(params) {
    declare_node();
    if (b && documentation(p) && identifier(p) && operator_(":") && type_expression(p)) return success(parent, self);
    return failure(save, self);
}

bool type_interface_declaration(params) {
    declare_node();
    if (b && documentation(p) && type_signature(p) && interface_declaration_block(p)) return success(parent, self);
    return failure(save, self);
}

bool kind_interface_declaration(params) {
    declare_node();
    if (b && documentation(p) && kind_signature(p)) return success(parent, self);
    return failure(save, self);
}

bool space_interface_declaration(params) {
    declare_node();
    if (b && documentation(p) && identifier(p) && interface_declaration_block(p)) return success(parent, self);
    return failure(save, self);
}

bool enum_interface_declaration(params) {
    declare_node();
    if (b && documentation(p) && identifier(p) && operator_(":") && type_expression(p) && enum_interface_declaration_block(p)) return success(parent, self);
    if (b && documentation(p) && identifier(p) && operator_(":") && enum_interface_declaration_block(p)) return success(parent, self);
    return failure(save, self);
}

bool using_type_assignment_statement(params) {
    declare_node();
    if (b && keyword_("using") && type_assignment_statement(p)) return success(parent, self);
    return failure(save, self);
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
    if (b && function_call_signature(p)) return success(parent, self);
    if (b && type_call_signature(p) && type_call_signature(p)) return success(parent, self);
    if (b && kind_call_signature(p) && kind_call_signature(p)) return success(parent, self);
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
    if (b && identifier(p) && filename_list(p) && filename_list(p)) return success(parent, self);
    if (b && filename_list(p)) return success(parent, self);
    if (b && identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool filename_list(params) {
    declare_node();
    if (b && filename(p) && filename_list(p) && filename_list(p)) return success(parent, self);
    if (b && filename(p)) return success(parent, self);
    return failure(save, self);
}

bool filename(params) {
    declare_node();
    if (b && operator_(".") && identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool function_implementation_declaration(params) {
    declare_node();
    if (b && function_signature(p) && block(p)) return success(parent, self);
    return failure(save, self);
}

bool variable_implementation_declaration(params) {
    declare_node();
    if (b && assignment_statement(p)) return success(parent, self);
    return failure(save, self);
}

bool type_implementation_declaration(params) {
    declare_node();
    if (b && type_signature(p) && implementation_declaration_block(p)) return success(parent, self);
    return failure(save, self);
}

bool kind_implementation_declaration(params) {
    declare_node();
    if (b && kind_signature(p) && block(p)) return success(parent, self);
    return failure(save, self);
}

bool space_implementation_declaration(params) {
    declare_node();
    if (b && identifier(p) && implementation_declaration_block(p)) return success(parent, self);
    return failure(save, self);
}

bool enum_implementation_declaration(params) {
    declare_node();
    if (b && identifier(p) && operator_(":") && type_expression(p) && enum_implementation_declaration_block(p)) return success(parent, self);
    if (b && identifier(p) && operator_(":") && enum_implementation_declaration_block(p)) return success(parent, self);
    return failure(save, self);
}

bool function_signature(params) {
    declare_node();
    if (b && function_call_signature(p) && return_type(p) && kind_call_signature(p) && precedence(p) && qualifier_list(p)) return success(parent, self);
    return failure(save, self);
}

bool type_signature(params) {
    declare_node();
    if (b && type_call_signature(p) && parent_type(p) && kind_call_signature(p) && precedence(p) && qualifier_list(p)) return success(parent, self);
    return failure(save, self);
}

bool kind_signature(params) {
    declare_node();
    if (b && kind_call_signature(p) && return_type(p) && precedence(p) && qualifier_list(p)) return success(parent, self);
    return failure(save, self);
}

bool function_call_signature(params) {
    declare_node();
    if (b && operator_("(") && parameter_list(p) && operator_(")")) return success(parent, self);
    return failure(save, self);
}

bool type_call_signature(params) {
    declare_node();
    if (b && operator_("{") && parameter_list(p) && operator_("}")) return success(parent, self);
    return failure(save, self);
}

bool kind_call_signature(params) {
    declare_node();
    if (b && operator_("[") && kind_parameter_list(p) && operator_("]")) return success(parent, self);
    return failure(save, self);
}

bool parent_type(params) {
    declare_node();
    if (b && operator_(":") && type_expression(p)) return success(parent, self);
    optional();
}

bool precedence(params) {
    declare_node();
    if (b && operator_("<") && function_call_signature(p) && operator_("+") && number(p) && operator_(">")) return success(parent, self);
    if (b && operator_("<") && type_call_signature(p) && operator_("+") && number(p) && operator_(">")) return success(parent, self);
    if (b && operator_("<") && kind_call_signature(p) && operator_("+") && number(p) && operator_(">")) return success(parent, self);
    if (b && operator_("<") && number(p) && operator_(">")) return success(parent, self);
    optional();
}

bool qualifier_list(params) {
    declare_node();
    if (b && qualifier(p) && qualifier_list(p)) return success(parent, self);
    optional();
}

bool return_type(params) {
    declare_node();
    if (b && operator_("-") && operator_(">") && type_expression(p)) return success(parent, self);
    optional();
}

bool parameter_list(params) {
    declare_node();
    if (b && parameter(p) && parameter_list(p)) return success(parent, self);
    optional();
}

bool parameter(params) {
    declare_node();
    if (b && operator_("(") && optional_parameter_list(p) && operator_(")")) return success(parent, self);
    if (b && required_parameter(p)) return success(parent, self);
    return failure(save, self);
}

bool optional_parameter_list(params) {
    declare_node();
    if (b && optional_parameter(p) && optional_parameter_list(p)) return success(parent, self);
    optional();
}

bool kind_parameter_list(params) {
    declare_node();
    if (b && kind_parameter(p) && kind_parameter_list(p)) return success(parent, self);
    optional();
}

bool kind_parameter(params) {
    declare_node();
    if (b && operator_("(") && optional_kind_parameter_list(p) && operator_(")")) return success(parent, self);
    if (b && required_kind_parameter(p)) return success(parent, self);
    return failure(save, self);
}

bool required_kind_parameter(params) {
    declare_node();
    if (b && identifier(p) && operator_(":") && type_expression(p)) return success(parent, self);
    if (b && kind_free_identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool optional_kind_parameter(params) {
    declare_node();
    if (b && identifier(p) && operator_(":") && type_expression(p) && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("=") && expression(p)) return success(parent, self);
    if (b && kind_free_identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool optional_kind_parameter_list(params) {
    declare_node();
    if (b && optional_kind_parameter(p) && optional_kind_parameter_list(p)) return success(parent, self);
    if (b && optional_kind_parameter(p)) return success(parent, self);
    return failure(save, self);
}

bool required_parameter(params) {
    declare_node();
    if (b && identifier(p) && operator_(":") && type_expression(p)) return success(parent, self);
    if (b && free_identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool optional_parameter(params) {
    declare_node();
    if (b && identifier(p) && operator_(":") && type_expression(p) && operator_("=") && expression(p)) return success(parent, self);
    if (b && identifier(p) && operator_("=") && expression(p)) return success(parent, self);
    return failure(save, self);
}

bool implementation_declaration_or_statement_list(params) {
    declare_node();
    if (b && implementation_declaration(p) && implementation_declaration_or_statement_list(p)) return success(parent, self);
    if (b && statement(p) && implementation_declaration_or_statement_list(p)) return success(parent, self);
    optional();
}

bool block(params) {
    declare_node();
    if (b && operator_("{") && statement_list(p) && operator_("}")) return success(parent, self);
    return failure(save, self);
}

bool code(params) {
    declare_node();
    if (b && block(p)) return success(parent, self);
    if (b && operator_(",") && newlines(p) && statement(p)) return success(parent, self);
    if (b && required_newlines(p) && statement(p)) return success(parent, self);
    return failure(save, self);
}

bool statement_list(params) {
    declare_node();
    if (b && newlines(p) && terminated_statement(p) && statement_list(p)) return success(parent, self);
    if (b && newlines(p)) return success(parent, self);
    optional();
}

bool statement(params) {
    declare_node();
    if (b && if_statement(p)) return success(parent, self);
    if (b && while_statement(p)) return success(parent, self);
    if (b && repeatwhile_statement(p)) return success(parent, self);
    if (b && return_statement(p)) return success(parent, self);
    if (b && for_statement(p)) return success(parent, self);
    if (b && type_assignment_statement(p)) return success(parent, self);
    if (b && assignment_statement(p)) return success(parent, self);
    if (b && block(p)) return success(parent, self);
    if (b && using_statement(p)) return success(parent, self);
    if (b && import_statement(p)) return success(parent, self);
    if (b && expression(p)) return success(parent, self);
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

bool type_assignment_statement(params) {
    declare_node();
    if (b && identifier(p) && operator_(":") && operator_("=") && type_expression(p)) return success(parent, self);
    return failure(save, self);
}

bool return_statement(params) {
    declare_node();
    if (b && keyword_("return") && expression(p)) return success(parent, self);
    if (b && keyword_("return")) return success(parent, self);
    return failure(save, self);
}

bool identifier_list(params) {
    declare_node();
    if (b && identifier(p) && operator_(",") && identifier_list(p)) return success(parent, self);
    if (b && identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool for_statement(params) {
    declare_node();
    if (b && keyword_("for") && identifier_list(p) && keyword_("in") && expression(p) && code(p)) return success(parent, self);
    return failure(save, self);
}

bool repeatwhile_statement(params) {
    declare_node();
    if (b && keyword_("repeat") && code(p) && keyword_("while") && expression(p)) return success(parent, self);
    return failure(save, self);
}

bool while_statement(params) {
    declare_node();
    if (b && keyword_("while") && expression(p) && code(p)) return success(parent, self);
    return failure(save, self);
}

bool if_head_statement(params) {
    declare_node();
    if (b && keyword_("if") && expression(p) && code(p)) return success(parent, self);
    return failure(save, self);
}

bool if_statement(params) {
    declare_node();
    if (b && if_head_statement(p) && else_if_statement_list(p) && else_statement(p)) return success(parent, self);
    return failure(save, self);
}

bool else_statement(params) {
    declare_node();
    if (b && newlines(p) && keyword_("else") && code(p)) return success(parent, self);
    return failure(save, self);
}

bool else_if_statement_list(params) {
    declare_node();
    if (b && newlines(p) && else_if_statement(p) && else_if_statement_list(p)) return success(parent, self);
    optional();
}

bool else_if_statement(params) {
    declare_node();
    if (b && keyword_("else") && if_head_statement(p)) return success(parent, self);
    optional();
}

bool expression(params) {
    declare_node();
    if (b && free_identifier_or_symbol_list(p)) return success(parent, self);
    return failure(save, self);
}

bool type_expression(params) {
    declare_node();
    if (b && operator_("(") && expression(p) && operator_(")") && operator_(")")) return success(parent, self);
    if (b && free_identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool free_identifier_or_symbol_list(params) {
    declare_node();
    if (b && free_identifier(p) && free_identifier_or_symbol_list(p)) return success(parent, self);
    if (b && symbol(p) && free_identifier_or_symbol_list(p)) return success(parent, self);
    optional();
}

bool symbol(params) {
    declare_node();
    if (b && operator_("(") && expression_list(p) && operator_(")")) return success(parent, self);
    if (b && operator_("[") && expression_list(p) && operator_("]")) return success(parent, self);
    if (b && operator_("{") && expression_list(p) && operator_("}")) return success(parent, self);
    if (b && number(p)) return success(parent, self);
    if (b && string(p)) return success(parent, self);
    if (b && identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool expression_list(params) {
    declare_node();
    if (b && expression(p) && operator_(",") && expression_list(p)) return success(parent, self);
    if (b && expression(p)) return success(parent, self);
    optional();
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
