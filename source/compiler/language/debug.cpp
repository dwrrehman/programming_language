//
//  debug.cpp
//  language
//
//  Created by Daniel Rehman on 1903063.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "debug.hpp"

#include "arguments.hpp"
#include "preprocessor.hpp"
#include "lexer.hpp"
#include "parser.hpp"


#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

// ----------------------- command line arguments debugging: ----------------------------


void debug_arguments(struct arguments args) {
    for (auto a : args.files) {
        std::cout << "file: " << a.name << "\n";
        std::cout << "data: \n:::" << a.data << ":::\n";
    }
    
    std::cout << std::boolalpha;
    std::cout << "error = " << args.error << std::endl;
    std::cout << "use interpreter = " << args.use_interpreter << std::endl;
    std::cout << "exec name = " << args.executable_name << std::endl;
}






// ---------------------------- preprocessor debugging functions: -------------------------------


const char* convert_pp_token_type_representation(enum pp_token_type type) {
    switch (type) {
        case pp_null_type: return "{null}";
        case pp_text_type: return "text";
        case pp_identifier_type: return "identifier";
        case pp_keyword_type: return "keyword";
        case pp_ast_node_type: return "astnode";
    }
}

void print_pp_lex(const std::vector<struct pp_token> &tokens) {
    std::cout << "::::::::::PP LEX:::::::::::" << std::endl;
    for (auto token : tokens) {
        std::cout << "TOKEN(type: " << convert_pp_token_type_representation(token.type) << ", value: \"" << (token.value != "\n" ? token.value : "\\n") << "\", [" << token.line << ":" << token.column << "])" << std::endl;
    }
    std::cout << ":::::::END OF PP LEX:::::::\n\n\n" << std::endl;
}

void print_pp_node(pp_node &self, int level) {
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

void print_pp_parse(pp_node &tree) {
    std::cout << "------------ PARSE: ------------- " << std::endl;
    print_pp_node(tree, 0);
}


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












// ---------------------------- lexer debugging: ---------------------------------------


void print_lex(const std::vector<struct token> &tokens) {
    std::cout << "::::::::::LEX:::::::::::" << std::endl;
    for (auto token : tokens) {
        std::cout << "TOKEN(type: " << convert_token_type_representation(token.type) << ", value: \"" << (token.value != "\n" ? token.value : "\\n") << "\", [" << token.line << ":" << token.column << "])" << std::endl;
    }
    std::cout << ":::::::END OF LEX:::::::" << std::endl;
}

const char* convert_token_type_representation(enum token_type type) {
    switch (type) {
        case token_type::null: return "{null}";
        case token_type::string: return "string";
        case token_type::identifier: return "identifier";
        case token_type::keyword: return "keyword";
        case token_type::operator_: return "operator";
        case token_type::documentation: return "documentation";
        case token_type::character: return "character";
        case token_type::llvm: return "llvm";
        case token_type::builtin: return "builtin";
        case token_type::indent: return "indent";
    }
}

void debug_token_stream() {
    std::vector<struct token> tokens = {};
    struct token t = {};
    while ((t = next()).type != token_type::null) tokens.push_back(t);
    print_lex(tokens);
}



// ---------------------------- parser -------------------------------


#define prep(_level) for (int i = _level; i--;) std::cout << ".   "


void print_expression_list(expression_list list) {
    std::cout << "pritning expression list:\n";
    for (auto e : list.expressions) {
        std::cout << "length: " << e.symbols.size() << "\n";
        std::cout << std::boolalpha << "error: " << e.error << "\n";
        std::cout << "\n";
    }
}
