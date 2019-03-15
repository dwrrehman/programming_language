//
//  preprocessor.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "preprocessor.hpp"
#include "lists.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <unordered_map>
#include <iostream>
#include <vector>
#include <exception>



// ----------------------------- pre-preprocessor -----------------------------------

std::string strip_comments(std::string text) {
    
    text.append("    ");
    
    std::string result = "";
    bool in_multi_comment = false;
    bool in_line_comment = false;
    
    for (int c = 0; c < text.size(); c++) {
        if (!in_line_comment && !in_multi_comment && text[c] == ';' && text[c+1] == ' ') {
            in_line_comment = true;
            result.push_back(' ');
            
        } else if (in_line_comment && !in_multi_comment && text[c] == '\n') {
            result.push_back('\n');
            in_line_comment = false;
            
        } else if (!in_line_comment && !in_multi_comment && text[c] == ';' && text[c+1] != ' ') {
            in_multi_comment = true;
            result.push_back(' ');
            
        } else if (!in_line_comment && in_multi_comment && text[c] == ';') {
            in_multi_comment = false;
            result.push_back(' ');
            
        } else if (text[c] == '\n') {
            result.push_back('\n');
            
        } else if (text[c] == '\t') {
            result.push_back(' ');
        
        } else if (!in_line_comment && !in_multi_comment) {
            result.push_back(text[c]);
    
        } else {
            result.push_back(' ');
        }
    }
    
    if (in_multi_comment) {
        std::cout << "Error: unterminated multiline comment." << std::endl;
        ///TODO: add call using the standard error printing class.
        throw "Unterminated multiline comment";
    }
    
    
    
    return result;
}

bool isnt_all_spaces(std::string s) {
    for (auto c : s)
        if (c != ' ') return true;
    return false;
}

// --------------------------- lexer ------------------------------

std::vector<struct pp_token> pp_lexer(std::string text) {
    
    std::vector<struct pp_token> tokens = {};
    
    bool inside_text = true;  // we start out assuming we are in the text, untill we encounter a keyword/identifier.
    bool inside_identifier = false;
    bool inside_ast_node = false;
    bool inside_keyword = false;
    bool inside_string = false;
    
    std::string current_identifier = "";
    std::string current_text = "";
    std::string current_ast_node = "";
    std::string current_keyword = "";
    
    size_t line = 1, column = 0;

    size_t current_line = line;
    size_t current_column = column;
    
    text.append(" ");
    
    for (int c = 0; c < text.size() - 1; c++) {
        column++;
        
        if (text[c] == '\"' && !inside_string) {
            inside_string = true;
            current_text += "\"";
            
        } else if (text[c] == '\"' && inside_string) {
            inside_string = false;
            current_text += "\"";
            
        } else if (text[c] == '\\' && inside_text && !inside_string) {   // found a astnode, identifier, or a keyword:
            
            if (text[c+1] == '\\' ) {
                current_text += "\\";
                c++;
                
            } else if (text[c+1] == '{') { // found the start of an identifier:
                
                if (current_text != "" && isnt_all_spaces(current_text)) {
                    tokens.push_back({pp_text_type, current_text, current_line, current_column});
                }
                inside_text = false;
                current_text = "";
                
                current_line = line;
                current_column = column;
                inside_identifier = true;
                c++;
                
            } else if (text[c+1] == '[') { // found the start of a ast_node:
                
                if (current_text != "" && isnt_all_spaces(current_text)){
                    tokens.push_back({pp_text_type, current_text, current_line, current_column});
                }
                inside_text = false;
                current_text = "";
                
                inside_ast_node = true;
                current_line = line;
                current_column = column;
                c++;
                
            } else {
                if (current_text != "" && isnt_all_spaces(current_text)){
                    tokens.push_back({pp_text_type, current_text, current_line, current_column});
                }
                inside_text = false;
                current_text = "";
                
                inside_keyword = true;
                current_line = line;
                current_column = column;
            }
        } else if (text[c] == '\\' && !inside_string) {
             if (text[c+1] == '}') { // found the end of an identifer:
                tokens.push_back({pp_identifier_type, current_identifier, current_line, current_column});
                inside_identifier = false;
                current_identifier = "";
                 
                inside_text = true;
                current_line = line;
                current_column = column;
                c++;
                
             } else if (text[c+1] == ']') { // found the end of a ast_node:
                 tokens.push_back({pp_ast_node_type, current_ast_node, current_line, current_column});
                 inside_ast_node = false;
                 current_ast_node = "";
                 
                 inside_text = true;
                 current_line = line;
                 current_column = column;
                 c++;
             }
            
        } else if (inside_keyword && (text[c] == ' ' || text[c] == '\n')) {
            tokens.push_back({pp_keyword_type, current_keyword, current_line, current_column});
            inside_keyword = false;
            current_keyword = "";
            
            inside_text = true;
            current_line = line;
            current_column = column;
            
        } else if (inside_text) {
            current_text += text[c];
            
        } else if (inside_keyword) {
            current_keyword += text[c];
            
        } else if (inside_ast_node) {
            current_ast_node += text[c];
            
        } else if (inside_identifier) {
            current_identifier += text[c];
            
        }
        if (text[c] == '\n') {
            line++;
            column = 0;
        }
    }

    if (current_text != "" && isnt_all_spaces(current_text)){
        tokens.push_back({pp_text_type, current_text, current_line, current_column});
    }
    
    
    
    return tokens;
}





/// Header for computer generated parser for boogers.

class postinformation {
public:
    
    postinformation(){}
};

/// Parser AST nodes:

class pp_node {
public:
    
    std::string name = "";
    int symbol_index = 0;
    bool success = false;
    struct pp_token data = {pp_null_type, "", 0, 0};
    postinformation post = {};
    std::vector<pp_node> children = {};
    
    pp_node(std::string name, struct pp_token data, std::vector<pp_node> children, bool success) {
        this->name = name;
        this->children = children;
        this->success = success;
        this->data = data;
    }
    pp_node(){}
};

class parse_error {
public:
    
    std::string expected = "";
    struct pp_token at = {pp_null_type, "",  0, 0};
    parse_error(std::string expected, struct pp_token data) {
        this->expected = expected;
        this->at = data;
    }
    parse_error(){}
};

class program parse(std::string filename, std::string text, std::vector<struct token> tokens, bool &error);
void print_node(pp_node &node, int level);



#define prep(_level) for (int i = _level; i--;) std::cout << ".   "

static void print_pp_node(pp_node &self, int level) {
    prep(level); std::cout << self.name << " (" << self.children.size() << ")" << std::endl;
    if (self.data.type != pp_null_type) {
        prep(level); std::cout << "type = " << convert_pp_token_type_representation(self.data.type) << std::endl;
    }
    if (self.data.value != "") {
        prep(level); std::cout << "value = " << self.data.value << std::endl;
    }
    int i = 0;
    for (auto childnode : self.children) {
        std::cout << std::endl;
        if (self.children.size() > 1) {prep(level+1); std::cout << "child #" << i++ << ": " << std::endl;}
        print_pp_node(childnode, level+1);
    }
}

//static void print_pp_parse(pp_node &tree) {
//    std::cout << "------------ PARSE: ------------- " << std::endl;
//    print_pp_node(tree, 0);
//}


/// Parsing:

static int pointer = 0;
static std::vector<pp_node> stack_trace = {};

static int deepest_pointer = 0;
static pp_node deepest_node = {};
static std::vector<pp_node> deepest_stack_trace = {};

size_t deepest_level = 0;
size_t level = 0;

#define declare_node()    pp_node self = pp_node(__func__, {}, {}, true);                       \
int save = pointer;                                             \
stack_trace.push_back(self);                                    \
level++;                                                        \

#define params            std::vector<struct pp_token> tokens, pp_node &parent
#define p                 tokens, self

#define optional() level--; stack_trace.pop_back(); return begin(save, self);

#define keyword_(kw)      terminal(pp_keyword_type, kw, p)

#define b                 begin(save, self)

static bool begin(int save, pp_node &self) {
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

static bool failure(int save, pp_node &self) {
    pointer = save;
    self.children.clear();
    level--;
    stack_trace.pop_back();
    return false;
}
static bool success(pp_node &parent, const pp_node &self) {
    parent.children.push_back(self);
    level--;
    stack_trace.pop_back();
    return true;
}
static bool push_terminal(pp_node &parent, std::vector<struct pp_token> &tokens) {
    parent.children.push_back(pp_node("terminal", tokens[pointer++], {}, true));
    if (level > deepest_level) {
        deepest_level = level;
        deepest_node = parent;
        deepest_stack_trace = stack_trace;
    }
    if (deepest_pointer < pointer)
        deepest_pointer = pointer;
    
    return true;
}

static bool terminal(enum pp_token_type desired_token_type, std::string desired_token_value, params) {
    if (level > deepest_level) {
        deepest_level = level;
        deepest_node = parent;
        deepest_stack_trace = stack_trace;
    }
    if (deepest_pointer < pointer)
        deepest_pointer = pointer;
    
    if (pointer >= tokens.size()) return false;
    if (desired_token_type == pp_identifier_type && tokens[pointer].type == pp_identifier_type) return push_terminal(parent, tokens);
    if (desired_token_type == pp_text_type && tokens[pointer].type == pp_text_type) return push_terminal(parent, tokens);
    
    
    if (desired_token_type == tokens[pointer].type && desired_token_value == tokens[pointer].value) return push_terminal(parent, tokens);
    return false;
}

/// Hand made EBNF nodes interfaces:

bool identifier(params);

bool raw_text(params);


//this is a automatically generated parser in cpp, for boogers.


bool program(params);

bool macro_list(params);

bool macro(params);

bool pattern(params);

bool pattern_element(params);

bool replacement(params);

bool statement_list(params);

bool statement(params);

bool block_statement(params);

bool if_statement(params);

bool let_statement(params);

bool assignment_statement(params);

bool emit_statement(params);

bool while_statement(params);

bool function_definition(params);

bool parameter_list(params);

bool parameter(params);

bool function_call(params);

bool expression_list(params);

bool expression(params);

bool and_expr(params);

bool compare_expr(params);

bool sum_expr(params);

bool product_expr(params);

bool unary_expr(params);

bool symbol(params);

bool number(params);




//EBNF Parse Nodes:

bool program(params) {
    declare_node();
    if (b && raw_text(p) && macro_list(p)) return success(parent, self);
    if (b && macro_list(p)) return success(parent, self);
    return failure(save, self);
}

bool macro_list(params) {
    declare_node();
    if (b && macro(p) && raw_text(p) && macro_list(p)) return success(parent, self);
    if (b && macro(p) && macro_list(p)) return success(parent, self);
    optional();
}

bool macro(params) {
    declare_node();
    if (b && keyword_("replace") && pattern(p) && keyword_("with") && replacement(p) && keyword_("end")) return success(parent, self);
    return failure(save, self);
}

bool pattern(params) {
    declare_node();
    if (b && pattern_element(p) && pattern(p)) return success(parent, self);
    optional();
}

bool pattern_element(params) {
    declare_node();
    if (b && raw_text(p)) return success(parent, self);
    if (b && identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool replacement(params) {
    declare_node();
    if (b && raw_text(p) && statement_list(p)) return success(parent, self);
    if (b && statement_list(p)) return success(parent, self);
    optional();
}

bool statement_list(params) {
    declare_node();
    if (b && statement(p) && raw_text(p) && statement_list(p)) return success(parent, self);
    if (b && statement(p) && statement_list(p)) return success(parent, self);
    optional();
}

bool statement(params) {
    declare_node();
    if (b && if_statement(p)) return success(parent, self);
    if (b && let_statement(p)) return success(parent, self);
    if (b && assignment_statement(p)) return success(parent, self);
    if (b && while_statement(p)) return success(parent, self);
    if (b && block_statement(p)) return success(parent, self);
    if (b && emit_statement(p)) return success(parent, self);
    if (b && function_definition(p)) return success(parent, self);
    if (b && function_call(p) && keyword_("end")) return success(parent, self);
    if (b && expression(p) && keyword_("end")) return success(parent, self);
    return failure(save, self);
}

bool block_statement(params) {
    declare_node();
    if (b && keyword_("begin") && replacement(p) && keyword_("end")) return success(parent, self);
    return failure(save, self);
}

bool if_statement(params) {
    declare_node();
    if (b && keyword_("if") && keyword_("(") && expression(p) && keyword_(")") && keyword_("do") && replacement(p) && keyword_("else") && replacement(p) && keyword_("end")) return success(parent, self);
    return failure(save, self);
}

bool let_statement(params) {
    declare_node();
    if (b && keyword_("let") && identifier(p) && keyword_("=") && expression(p) && keyword_("end")) return success(parent, self);
    if (b && keyword_("let") && keyword_("int") && identifier(p) && keyword_("=") && expression(p) && keyword_("end")) return success(parent, self);
    return failure(save, self);
}

bool assignment_statement(params) {
    declare_node();
    if (b && identifier(p) && keyword_("=") && expression(p) && keyword_("end")) return success(parent, self);
    return failure(save, self);
}

bool emit_statement(params) {
    declare_node();
    if (b && keyword_("emit") && expression(p) && keyword_("end")) return success(parent, self);
    return failure(save, self);
}

bool while_statement(params) {
    declare_node();
    if (b && keyword_("while") && keyword_("(") && expression(p) && keyword_(")") && keyword_("do") && replacement(p) && keyword_("end")) return success(parent, self);
    return failure(save, self);
}

bool function_definition(params) {
    declare_node();
    if (b && keyword_("define") && identifier(p) && keyword_("(") && parameter_list(p) && keyword_(")") && block_statement(p)) return success(parent, self);
    return failure(save, self);
}

bool parameter_list(params) {
    declare_node();
    if (b && parameter(p) && keyword_(",") && parameter_list(p)) return success(parent, self);
    if (b && parameter(p)) return success(parent, self);
    optional();
}

bool parameter(params) {
    declare_node();
    if (b && keyword_("let") && keyword_("int") && identifier(p)) return success(parent, self);
    if (b && keyword_("let") && identifier(p)) return success(parent, self);
    return failure(save, self);
}

bool function_call(params) {
    declare_node();
    if (b && keyword_("call") && identifier(p) && keyword_("(") && expression_list(p) && keyword_(")")) return success(parent, self);
    return failure(save, self);
}

bool expression_list(params) {
    declare_node();
    if (b && expression(p) && keyword_(",") && expression_list(p)) return success(parent, self);
    if (b && expression(p)) return success(parent, self);
    optional();
}

bool expression(params) {
    declare_node();
    if (b && and_expr(p) && keyword_("|") && and_expr(p)) return success(parent, self);
    if (b && and_expr(p)) return success(parent, self);
    return failure(save, self);
}

bool and_expr(params) {
    declare_node();
    if (b && compare_expr(p) && keyword_("&") && compare_expr(p)) return success(parent, self);
    if (b && compare_expr(p)) return success(parent, self);
    return failure(save, self);
}

bool compare_expr(params) {
    declare_node();
    if (b && sum_expr(p) && keyword_("==") && sum_expr(p)) return success(parent, self);
    if (b && sum_expr(p) && keyword_("<") && sum_expr(p)) return success(parent, self);
    if (b && sum_expr(p) && keyword_(">") && sum_expr(p)) return success(parent, self);
    if (b && sum_expr(p)) return success(parent, self);
    return failure(save, self);
}

bool sum_expr(params) {
    declare_node();
    if (b && product_expr(p) && keyword_("+") && product_expr(p)) return success(parent, self);
    if (b && product_expr(p) && keyword_("-") && product_expr(p)) return success(parent, self);
    if (b && product_expr(p)) return success(parent, self);
    return failure(save, self);
}

bool product_expr(params) {
    declare_node();
    if (b && unary_expr(p) && keyword_("*") && unary_expr(p)) return success(parent, self);
    if (b && unary_expr(p) && keyword_("/") && unary_expr(p)) return success(parent, self);
    if (b && unary_expr(p)) return success(parent, self);
    return failure(save, self);
}

bool unary_expr(params) {
    declare_node();
    if (b && keyword_("!") && symbol(p) && symbol(p)) return success(parent, self);
    if (b && symbol(p)) return success(parent, self);
    return failure(save, self);
}

bool symbol(params) {
    declare_node();
    if (b && keyword_("(") && expression(p) && keyword_(")")) return success(parent, self);
    if (b && number(p)) return success(parent, self);
    if (b && function_call(p)) return success(parent, self);
    if (b && identifier(p)) return success(parent, self);
    if (b && raw_text(p)) return success(parent, self);
    return failure(save, self);
}

bool number(params) {
    declare_node();
    if (b && keyword_("int") && identifier(p)) return success(parent, self);
    if (b && keyword_("int") && keyword_("+") && identifier(p)) return success(parent, self);
    if (b && keyword_("int") && keyword_("-") && identifier(p)) return success(parent, self);
    return failure(save, self);
}

/// Hand made EBNF nodes:

bool identifier(params){
    declare_node();
    if (begin(save, self) && terminal(pp_identifier_type, "", p)) return success(parent, self);
    return failure(save, self);
}

bool raw_text(params){
    declare_node();
    if (begin(save, self) && terminal(pp_text_type, "", p)) return success(parent, self);
    return failure(save, self);
}

pp_node pp_parser(std::string filename, std::vector<struct pp_token> tokens) {
    pp_node tree = {};
    if (!program(tokens, tree) || pointer != tokens.size() || level) {
        int i = 0;
        for (auto n : deepest_stack_trace) print_pp_node(n, i++);
        if (deepest_pointer >= tokens.size()) deepest_pointer--;
        print_parse_error(filename, tokens[deepest_pointer].line,  tokens[deepest_pointer].column, convert_pp_token_type_representation(tokens[deepest_pointer].type), tokens[deepest_pointer].value);
        throw "parse error";
    }
    return tree;
}


enum value_type {
    null_value_type,
    int_value_type,
    text_value_type,
    function_value_type
};

struct value {
    int numeric = 0;
    std::string textual = "";
    pp_node* function_definition = nullptr;
    std::unordered_map<std::string, struct value> call_scope;
    enum value_type type = null_value_type;
};


void print_value(struct value v);

void print_current_symbol_table(std::unordered_map<std::string, struct value> symbol_table) {
    std::cout << "SYMBOL TABLE:" << std::endl;
    
    if (!symbol_table.size()) {
        printf("\t{EMPTY}\n");
        return;
    }
    std::cout << "--------------------------------------------------\n";
    for (auto elem : symbol_table) {
        std::cout << "[" << elem.first << "] :: ";
        print_value(elem.second);
    }
    std::cout << "--------------------------------------------------\n\n";
}

void print_value(struct value v) {
    std::cout << "VALUE(numeric: " << v.numeric << ", textual: " << v.textual << ", node: " << v.function_definition << ", type: " << v.type << ")" << std::endl;
    if (v.type == function_value_type) {
        std::cout << "printing call scope:" << std::endl;
        print_current_symbol_table(v.call_scope);
    }
}


void print_symbol_table_stack(std::vector<std::unordered_map<std::string, struct value>> symbol_table_stack) {
    std::cout << "---------------- printing stack ----------------------\n";
    for (auto s : symbol_table_stack) print_current_symbol_table(s);
    std::cout << "----------------done printing ----------------------\n";
}

struct value evaluate_integer_binary_expression(const struct value &first, const struct value &second, int result) {
    if (first.type == int_value_type && second.type == int_value_type) {
        return {result, "", nullptr, {}, int_value_type};
    } else {
        printf("Error: type Mismatch!\n");
        printf("{first type: %d, second type: %d}\n", first.type, second.type);
        return {};
    }
}


void interpret(pp_node &tree, std::vector<std::unordered_map<std::string, struct value>> &symbol_table_stack, std::string &text);


struct value evaluate(pp_node expression, std::unordered_map<std::string, struct value> &symbol_table, std::string& text) {
    
    if (expression.name == "raw_text") {
        auto string = expression.children[0].data.value;
        return {0, string, nullptr, {}, text_value_type};
        
    } else if (expression.name == "number") {
        auto number = expression.children[1].children[0].data.value;
        int value = std::atoi(number.c_str());
        if (!value && number != "0") {
            printf("Error: Expected a integer numeric literal after int keyword.\n");
            return {0, "", nullptr, {}, null_value_type}; // error
        } else return {value, "", nullptr, {}, int_value_type};
            
    } else if (expression.name == "identifier") {
        
        const auto name = expression.children[0].data.value;
        const auto entry = symbol_table.find(name);
        if (entry != symbol_table.end())
            return entry->second;
        else {
            printf("Error: Undeclared variable \"%s\"\n", name.c_str());
            return {};
        }
    } else if (expression.name == "function_call") {
        const auto name = expression.children[1].children[0].data.value;
        const auto function = symbol_table.find(name);
        printf("calling a function....\n");
        print_current_symbol_table(symbol_table);
        if (function != symbol_table.end() && function->second.function_definition) {
            printf("function available!....\n");
            std::vector<std::unordered_map<std::string, struct value>> scope = {function->second.call_scope};
            std::cout << "usiong this scope:\n";
            print_symbol_table_stack(scope);
            interpret(*(function->second.function_definition), scope, text);
        } else {
            printf("no valid call to function \"%s\"\n", name.c_str());
        }
        
    } else if (expression.name == "sum_expr" && expression.children.size() == 3) {
        
        const struct value first = evaluate(expression.children[0], symbol_table, text);
        const struct value second = evaluate(expression.children[2], symbol_table, text);
        const std::string the_operator = expression.children[1].data.value;
        
        if (the_operator == "+") {
            if (first.type == int_value_type && second.type == int_value_type) return {first.numeric + second.numeric, "", nullptr, {}, int_value_type};
            else if (first.type == text_value_type && second.type == text_value_type) return {0, first.textual + second.textual, nullptr, {}, text_value_type};
            else {
                printf("Error: type Mismatch!\n");
                printf("{first type: %d, second type: %d}\n", first.type, second.type);
                return {};
            }
        } else if (the_operator == "-") {
            return evaluate_integer_binary_expression(first, second, first.numeric - second.numeric);
        }
        
    } else if (expression.name == "product_expr" && expression.children.size() == 3) {
        
        const struct value first = evaluate(expression.children[0], symbol_table, text);
        const struct value second = evaluate(expression.children[2], symbol_table, text);
        const std::string the_operator = expression.children[1].data.value;
        
        if (the_operator == "*") {
            return evaluate_integer_binary_expression(first, second, first.numeric * second.numeric);
            
        } else if (the_operator == "/") {
            if (!second.numeric) { printf("Error: Division by zero!\n"); return {}; }
            return evaluate_integer_binary_expression(first, second, first.numeric / second.numeric);
        }
        
    } else if (expression.name == "expression" && expression.children.size() == 3) {
        
        const struct value first = evaluate(expression.children[0], symbol_table, text);
        const struct value second = evaluate(expression.children[2], symbol_table, text);
        return evaluate_integer_binary_expression(first, second, first.numeric || second.numeric);
        
    } else if (expression.name == "and_expr" && expression.children.size() == 3) {
        
        const struct value first = evaluate(expression.children[0], symbol_table, text);
        const struct value second = evaluate(expression.children[2], symbol_table, text);
        return evaluate_integer_binary_expression(first, second, first.numeric && second.numeric);
        
    } else if (expression.name == "compare_expr" && expression.children.size() == 3) {
        
        const struct value first = evaluate(expression.children[0], symbol_table, text);
        const struct value second = evaluate(expression.children[2], symbol_table, text);
        const std::string the_operator = expression.children[1].data.value;
        
        if (the_operator == "==") {
            return evaluate_integer_binary_expression(first, second, first.numeric == second.numeric);
            
        } else if (the_operator == "<") {
            return evaluate_integer_binary_expression(first, second, first.numeric < second.numeric);
            
        } else if (the_operator == ">") {
            return evaluate_integer_binary_expression(first, second, first.numeric > second.numeric);
        }
    }
    
    if (expression.children.size())
        return evaluate(expression.children[0], symbol_table, text);
    else return {};
}

void push_arguments_into_symbol_table(pp_node &parameter_list, std::unordered_map<std::string, struct value> &symbol_table) {
    if (parameter_list.name == "parameter") {
        auto name = parameter_list.children.back().children[0].data.value;
        auto type = parameter_list.children.size() > 2 ? int_value_type : text_value_type;
        symbol_table[name] = {0, "", nullptr, {}, type};
    }
    for (auto child : parameter_list.children) {
        push_arguments_into_symbol_table(child, symbol_table);
    }
}

void interpret(pp_node &tree, std::vector<std::unordered_map<std::string, struct value>> &symbol_table_stack, std::string &text) {

    if (tree.name == "block_statement") {
        symbol_table_stack.push_back(symbol_table_stack.back());
        std::cout << "ENCOUNTERED A BLOCK STATEMENT!\n";
        
    } else if (tree.name == "function_definition") {
        
        const auto name = tree.children[1].children[0].data.value;
        symbol_table_stack.back()[name] = {0, "", &tree.children.back(), {}, function_value_type};
        
        if (tree.children.size() > 5) {
            symbol_table_stack.push_back(symbol_table_stack.back());
            
            auto parameter_list = tree.children[3];
            push_arguments_into_symbol_table(parameter_list, symbol_table_stack.back());
            std::unordered_map<std::string, struct value> saved_scope = symbol_table_stack.back();
            
            symbol_table_stack.pop_back();
            symbol_table_stack.back()[name].call_scope = saved_scope;
        }
        return;
        
    } else if (tree.name == "let_statement") {
        auto identifier = tree.children[1].children[0].data.value;
        symbol_table_stack.back()[identifier] = evaluate(tree.children[3], symbol_table_stack.back(), text);
        
    } else if (tree.name == "assignment_statement") {
        auto identifier = tree.children[0].children[0].data.value;
        symbol_table_stack.back()[identifier] = evaluate(tree.children[2], symbol_table_stack.back(), text);
        
    } else if (tree.name == "if_statement") {
        auto value = evaluate(tree.children[2], symbol_table_stack.back(), text);
        if (!value.numeric) return;
        symbol_table_stack.push_back(symbol_table_stack.back());
        
    } else if (tree.name == "while_statement") {
        symbol_table_stack.push_back(symbol_table_stack.back());
        while_statement_label:
        auto value = evaluate(tree.children[2], symbol_table_stack.back(), text);
        if (!value.numeric) {
            symbol_table_stack.pop_back();
            return;
        }
    
    } else if (tree.name == "emit_statement") {
        auto value = evaluate(tree.children[1], symbol_table_stack.back(), text);
        if (value.type == int_value_type) std::cout << value.numeric;
        else if (value.type == text_value_type) std::cout << value.textual;
        else printf("{EMIT ERROR}");
        
    } else if (tree.name == "expression") {
        evaluate(tree, symbol_table_stack.back(), text);
        
    } else if (tree.name == "function_call") {
        evaluate(tree, symbol_table_stack.back(), text);
    }
    
    for (auto child : tree.children) {
        interpret(child, symbol_table_stack, text);
    }
    
    if (tree.name == "block_statement") {
        symbol_table_stack.pop_back();
        
    } else if (tree.name == "function_definition") {
        symbol_table_stack.pop_back();
        
    } else if (tree.name == "if_statement") {
        symbol_table_stack.pop_back();
        
    } else if (tree.name == "while_statement") {        
        goto while_statement_label;
    }
}

std::string preprocess(std::string filename, std::string text) {
    
    //std::cout << "---------orginal text:----------\n:::" << text << ":::\n\n\n";
    
    std::vector<std::unordered_map<std::string, struct value>> symbol_table_stack = {{}};
    
    text = strip_comments(text);
    
    //auto tokens = pp_lexer(text);
    //print_pp_lex(tokens);
    
    //auto action_tree = pp_parser(filename, tokens);
    //print_pp_parse(action_tree);
    
    //interpret(action_tree, symbol_table_stack, text);
        
    //std::cout << "-------preprocessed text:--------\n:::" << text << ":::\n\n\n";
    
    return text; // DEBUG: CHANGE ME
}
