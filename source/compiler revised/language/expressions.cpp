//
//  expressions.cpp
//  language
//
//  Created by Daniel Rehman on 1902096.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>

#include "expressions.hpp"
#include "parser.hpp"
#include "lists.hpp"


/// Expression Debugger:

#define prep(_level) for (int i = _level; i--;) std::cout << ".   "

void print_expression_node(expression_node &self, int level) {
    prep(level); std::cout << self.name << std::endl;
    if (self.data.type != null_type) {
        prep(level); std::cout << "type = " << convert_token_type_representation(self.data.type) << std::endl;
    }
    if (self.data.value != "") {
        prep(level); std::cout << "value = " << self.data.value << std::endl;
    }
    int i = 0;
    for (auto child : self.children) {
        std::cout << std::endl;
        if (self.children.size() > 1) {prep(level+1); std::cout << "child #" << i++ << ": " << std::endl;}
        print_expression_node(child, level+1);
    }
}

void print_expression_parse(expression_node &tree) {
    std::cout << "----------------------- EXPRESSION PARSE: ---------------------- " << std::endl;
    print_expression_node(tree, 0);
}


void print_expression_token(struct token t) {
    std::cout << "Error at token: \n\n";
    std::cout << "\t\t---------------------------------\n";
    std::cout << "\t\tline " << t.line << ": " << t.value << "           "  <<  "(" << convert_token_type_representation(t.type) << ")\n";
    std::cout << "\t\t---------------------------------\n\n\n";
}




/// Expression Tokenizer:

void tokenize_subtree(node tree, std::vector<node> &result) {
    if (tree.name == "terminal") {
        result.push_back(tree);
        return;
    }
    for (auto child : tree.children)
        tokenize_subtree(child, result);
}

static std::vector<node> tokenize(node input) {
    if (input.name != "expression") {
        std::cout << "received something thats not an expression. aborting...\n";
        return {};
    }

    print_node(input, 0);
    
    std::vector<node> result = {};
    tokenize_subtree(input, result);
    return result;
}




/// Expression Parser:

static int pointer = 0;

#define declare_node()    expression_node self = expression_node(__func__, {}, {}); int save = pointer; back_trace.push_back(self);
#define params            std::vector<node> tokens, expression_node &parent, std::vector<expression_node> &back_trace, std::vector<struct function_signature> signatures
#define p                 tokens, self, back_trace, signatures

static bool begin(int save, expression_node &self) {
    pointer = save;
    self.children.clear();
    return true;
}
static bool failure(int save, expression_node &self) {
    pointer = save;
    self.children.clear();
    
    return false;
}
static bool success(expression_node &parent, const expression_node &self) {
    parent.children.push_back(self);
    return true;
}
static bool push_terminal(expression_node &parent, std::vector<node> &tokens) {
    parent.children.push_back(expression_node("terminal", tokens[pointer++].data, {}));
    return true;
}

static bool terminal(enum token_type desired_token_type, std::string desired_token_value, params) {
    if (pointer >= tokens.size()) return false;
    if (desired_token_type == identifier_type && tokens[pointer].data.type == identifier_type) return push_terminal(parent, tokens);
    if (desired_token_type == string_type && tokens[pointer].data.type == string_type) return push_terminal(parent, tokens);
    if (desired_token_type == documentation_type && tokens[pointer].data.type == documentation_type) return push_terminal(parent, tokens);
    if (desired_token_type == number_type && tokens[pointer].data.type == number_type) return push_terminal(parent, tokens);
    if (desired_token_type == tokens[pointer].data.type && desired_token_value == tokens[pointer].data.value) return push_terminal(parent, tokens);
    return false;
}

static bool identifier(params){
    declare_node();
    if (begin(save, self) && terminal(identifier_type, "", p)) return success(parent, self);
    return failure(save, self);
}

static bool symbol(params) {
    declare_node();
    if (begin(save, self) && terminal(number_type, "", p)) return success(parent, self);
    if (begin(save, self) && terminal(string_type, "", p)) return success(parent, self);
    if (begin(save, self) && identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool expression(params);
bool expression_list(params);


// unimplemented:
bool typecall(params) {
    declare_node();
    if (begin(save, self) ) return success(parent, self);
    return failure(save, self);
}


bool functioncall(params) {
    declare_node();
    for (auto signature : signatures) {
        bool matches = true;
        for (auto element : signature.call_signature) {
            
        }
        if (begin(save, self) && matches) return success(parent, self);
    }
    
    return failure(save, self);
}

bool where_clause_expression(params) {
    declare_node();
    if (begin(save, self) && terminal(keyword_type, "where", p) && expression(p)) return success(parent, self);
    return begin(save, self);
}

bool symbol_expression(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, "[", p) && expression_list(p) && terminal(operator_type, "]", p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "[", p) && expression(p) && terminal(keyword_type, "for", p) && expression(p) && terminal(keyword_type, "in", p) && expression(p) && where_clause_expression(p) && terminal(operator_type, "]", p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "(", p) && expression_list(p) && terminal(operator_type, ")", p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "(", p) && expression(p) && terminal(operator_type, ")", p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "{", p) && typecall(p) && terminal(operator_type, "}", p)) return success(parent, self);
    if (begin(save, self) && functioncall(p)) return success(parent, self);
    if (begin(save, self) && symbol(p)) return success(parent, self);
    return failure(save, self);
}

bool access_expression(params) {
    declare_node();
    if (begin(save, self) && symbol_expression(p) && terminal(operator_type, "[", p) && expression(p) && terminal(operator_type, "]", p)) success(parent, self);
    if (begin(save, self) && symbol_expression(p) && terminal(operator_type, "*", p)) success(parent, self);
    if (begin(save, self) && symbol_expression(p) && terminal(operator_type, "&", p)) success(parent, self);
    if (begin(save, self) && symbol_expression(p) && terminal(operator_type, ".", p)) success(parent, self);
    if (begin(save, self) && symbol_expression(p)) success(parent, self);
    return failure(save, self);
}

bool unary_expression(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, "|", p) && access_expression(p) && terminal(operator_type, "|", p)) return success(parent, self);
    if (begin(save, self) && terminal(keyword_type, "not", p) && access_expression(p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "!", p) && access_expression(p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "~", p) && access_expression(p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "+", p) && access_expression(p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "-", p) && access_expression(p)) return success(parent, self);
    if (begin(save, self) && access_expression(p)) return success(parent, self);
    return failure(save, self);
}

bool product_expression_list(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, "*", p) && unary_expression(p) && product_expression_list(p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "/", p) && unary_expression(p) && product_expression_list(p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "%", p) && unary_expression(p) && product_expression_list(p)) return success(parent, self);
    return begin(save, self);
}

bool product_expression(params) {
    declare_node();
    if (begin(save, self) && unary_expression(p) && product_expression_list(p)) return success(parent, self);
    return failure(save, self);
}

bool sum_expression_list(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, "+", p) && product_expression(p) && sum_expression_list(p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "-", p) && product_expression(p) && sum_expression_list(p)) return success(parent, self);
    return begin(save, self);
}

bool sum_expression(params) {
    declare_node();
    if (begin(save, self) && product_expression(p) && sum_expression_list(p)) return success(parent, self);
    return failure(save, self);
}

bool bitwise_shift_expression_list(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, ">", p) && terminal(operator_type, ">", p) && sum_expression(p) && bitwise_shift_expression_list(p)) return success(parent, self);
    if (begin(save, self) && terminal(operator_type, "<", p) && terminal(operator_type, "<", p) && sum_expression(p) && bitwise_shift_expression_list(p)) return success(parent, self);
    return begin(save, self);
}

bool bitwise_shift_expression(params) {
    declare_node();
    if (begin(save, self) && sum_expression(p) && bitwise_shift_expression_list(p)) return success(parent, self);
    return failure(save, self);
}

bool bitwise_and_expression_list(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, "&", p) && bitwise_shift_expression(p) && bitwise_and_expression_list(p)) return success(parent, self);
    return begin(save, self);
}

bool bitwise_and_expression(params) {
    declare_node();
    if (begin(save, self) && bitwise_shift_expression(p) && bitwise_and_expression_list(p)) return success(parent, self);
    return failure(save, self);
}

bool bitwise_xor_expression_list(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, "^", p) && bitwise_and_expression(p) && bitwise_xor_expression_list(p)) return success(parent, self);
    return begin(save, self);
}

bool bitwise_xor_expression(params) {
    declare_node();
    if (begin(save, self) && bitwise_and_expression(p) && bitwise_xor_expression_list(p)) return success(parent, self);
    return failure(save, self);
}

bool bitwise_or_expression_list(params) {
    declare_node();
    if (begin(save, self) && terminal(operator_type, "|", p) && bitwise_xor_expression(p) && bitwise_or_expression_list(p)) return success(parent, self);
    return begin(save, self);
}

bool bitwise_or_expression(params) {
    declare_node();
    if (begin(save, self) && bitwise_xor_expression(p) && bitwise_or_expression_list(p)) return success(parent, self);
    return failure(save, self);
}

bool comparison(params) {
    declare_node();
    if (begin(save, self) && bitwise_or_expression(p) && terminal(operator_type, "=", p) && terminal(operator_type, "=", p) && bitwise_or_expression(p)) return success(parent, self);
    if (begin(save, self) && bitwise_or_expression(p) && terminal(operator_type, "!", p) && terminal(operator_type, "=", p) && bitwise_or_expression(p)) return success(parent, self);
    if (begin(save, self) && bitwise_or_expression(p) && terminal(operator_type, "<", p) && terminal(operator_type, "=", p) && bitwise_or_expression(p)) return success(parent, self);
    if (begin(save, self) && bitwise_or_expression(p) && terminal(operator_type, ">", p) && terminal(operator_type, "=", p) && bitwise_or_expression(p)) return success(parent, self);
    if (begin(save, self) && bitwise_or_expression(p) && terminal(operator_type, ">", p)  && bitwise_or_expression(p)) return success(parent, self);
    if (begin(save, self) && bitwise_or_expression(p) && terminal(operator_type, "<", p)  && bitwise_or_expression(p)) return success(parent, self);
    if (begin(save, self) && bitwise_or_expression(p)) return success(parent, self);
    return failure(save, self);
}

bool logical_and_expression_list(params) {
    declare_node();
    if (begin(save, self) && terminal(keyword_type, "and", p) && comparison(p) && logical_and_expression_list(p)) return success(parent, self);
    if (begin(save, self) && terminal(keyword_type, "&", p) && terminal(keyword_type, "&", p) && comparison(p) && logical_and_expression_list(p)) return success(parent, self);
    return begin(save, self);
}

bool logical_and_expression(params) {
    declare_node();
    if (begin(save, self) && comparison(p) && logical_and_expression_list(p)) return success(parent, self);
    return failure(save, self);
}

bool logical_or_expression_list(params) {
    declare_node();
    if (begin(save, self) && terminal(keyword_type, "or", p) && logical_and_expression(p) && logical_or_expression_list(p)) return success(parent, self);
    if (begin(save, self) && terminal(keyword_type, "|", p) && terminal(keyword_type, "|", p) && logical_and_expression(p) && logical_or_expression_list(p)) return success(parent, self);
    return begin(save, self);
}

bool logical_or_expression(params) {
    declare_node();
    if (begin(save, self) && logical_and_expression(p) && logical_or_expression_list(p)) return success(parent, self);
    return failure(save, self);
}

bool conditional(params) {
    declare_node();
    if (begin(save, self) && terminal(keyword_type, "if", p) && expression(p) && terminal(operator_type, ",", p) && expression(p) && terminal(keyword_type, "else", p) && expression(p)) return success(parent, self);
    return failure(save, self);
}

bool expression(params) {
    declare_node();
    if (begin(save, self) && conditional(p)) return success(parent, self);
    if (begin(save, self) && logical_or_expression(p)) return success(parent, self);
    return failure(save, self);
}

bool expression_list(params) {
    declare_node();
    if (begin(save, self) && expression(p) && terminal(operator_type, ",", p) && expression_list(p)) return success(parent, self);
    if (begin(save, self) && expression(p) && expression_list(p)) return success(parent, self);
    return begin(save, self);
}



//// This is where we need to write a EBNF grammar
//// for our expressions, which encodes our precedence.

// lets do that.





expression_node parse_expression(node input, std::vector<struct function_signature> current_signatures) {
    
    std::vector<node> tokens = tokenize(input);
    
    expression_node tree = {};
    bool parse_successful = false;
    std::vector<expression_node> trace = {};
    
    if (!expression(tokens, tree, trace, current_signatures) || pointer != tokens.size()) {
        std::cout << "Error: parsing failed." << std::endl;
    } else parse_successful = true;
    
    print_expression_parse(tree);
    
    if (parse_successful) {
        std::cout << "\n\n\n\t\tsuccessfully parsed expresssion.\n\n\n" << std::endl;
    } else {
        std::cout << "\n\nFAILURE.\n\n\n" << std::endl;
        print_expression_token(tokens[pointer].data);
    }
    
    
    return tree;
}
