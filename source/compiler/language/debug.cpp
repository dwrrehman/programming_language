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
        std::cout << "data: \n:::" << a.text << ":::\n";
    }
    
    std::cout << std::boolalpha;
    std::cout << "error = " << args.error << std::endl;
    std::cout << "use interpreter = " << args.use_interpreter << std::endl;
    std::cout << "exec name = " << args.executable_name << std::endl;
}






// ---------------------------- preprocessor debugging functions: -------------------------------













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

void print_symbol(symbol s, int d);
void print_expression(expression s, int d);
void print_block(block b, int d);

void print_block(block block, int d) {
    prep(d); std::cout << "block:\n";
    print_expression_list(block.list, d+1);
}

void print_abstraction_definition(abstraction_definition abstraction, int d) {
    prep(d); std::cout << "abstraction definition: \n";

    prep(d+1); std::cout << "call signature: \n";
    print_expression(abstraction.call, d+2);

    prep(d+1); std::cout << "return type: \n";
    print_expression(abstraction.return_type, d+2);

    prep(d+1); std::cout << "signature type: \n";
    print_expression(abstraction.signature_type, d+2);

    prep(d+1); std::cout << "abstraction body: \n";
    print_block(abstraction.body, d+2);
}

void print_variable_definition(variable_definition variable, int d) {
    prep(d); std::cout << "variable definition: \n";

    prep(d+1); std::cout << "variable name: \n";
    print_expression(variable.name, d+2);

    prep(d+1); std::cout << "variable type: \n";
    print_expression(variable.type, d+2);
}

void print_symbol(symbol symbol, int d) {
    prep(d); std::cout << "symbol: \n";
    switch (symbol.type) {

        case symbol_type::identifier:
            prep(d); std::cout << convert_token_type_representation(symbol.identifier.name.type) << ": " << symbol.identifier.name.value << "\n";
            break;

        case symbol_type::llvm_literal:
            prep(d); std::cout << "llvm literal: \'" << symbol.llvm.literal.value << "\'\n";
            break;

        case symbol_type::string_literal:
            prep(d); std::cout << "string literal: \"" << symbol.string.literal.value << "\"\n";
            break;

        case symbol_type::character_literal:
            prep(d); std::cout << "chararacter literal: \'" << symbol.character.literal.value << "\'\n";
            break;

        case symbol_type::documentation:
            prep(d); std::cout << "documentation: `" << symbol.documentation.literal.value << "`\n";
            break;

        case symbol_type::subexpression:
            prep(d); std::cout << "sub expr: \n";
            print_expression(symbol.subexpression, d+1);
            break;

        case symbol_type::block:
            prep(d); std::cout << "block symbol\n";
            print_block(symbol.block, d+1);
            break;

        case symbol_type::newline:
            prep(d); std::cout << "{newline}\n"; 
            break;
        case symbol_type::indent:
            prep(d); std::cout << "{indent}\n";
            break;

        case symbol_type::none:
            prep(d); std::cout << "{NO SYMBOL TYPE}\n";
            break;
        case symbol_type::abstraction_definition:
            prep(d); std::cout << "abstraction definition: \n";
            print_abstraction_definition(symbol.abstraction, d+1);
            break;
        case symbol_type::variable_definition:
            prep(d); std::cout << "variable definition: \n";
            print_variable_definition(symbol.variable, d+1);
            break;
    }
}

void print_expression(expression expression, int d) {
    prep(d); std::cout << "expression: \n";
    prep(d); std::cout << std::boolalpha << "error: " << expression.error << "\n";
    prep(d); std::cout << "indent level = " << expression.indent_level << "\n";
    prep(d); std::cout << "symbol count: " << expression.symbols.size() << "\n";
    prep(d); std::cout << "symbols: \n";
    int i = 0;
    for (auto symbol : expression.symbols) {
        prep(d+1); std::cout << i << ": \n";
        print_symbol(symbol, d+1);
        std::cout << "\n";
        i++;
    }
}

void print_expression_list(expression_list list, int d) {

    prep(d); std::cout << "expression list:\n";
    prep(d); std::cout << "expression count = " << list.expressions.size() << "\n";
    int i = 0;
    for (auto e : list.expressions) {
        prep(d+1); std::cout << "expression #" << i << "\n";
        prep(d+1); std::cout << std::boolalpha << "error: " << e.error << "\n";
        print_expression(e, d+2);
        prep(d+1); std::cout << "\n";
        i++;
    }
}

void print_translation_unit(translation_unit unit, struct file file) {
    std::cout << "translation unit: (" << file.name << ")\n";
    print_expression_list(unit.list, 1);
}

std::string convert_symbol_type(enum symbol_type type) {
    switch (type) {
        case symbol_type::none:
            return "{none}";        
        case symbol_type::subexpression:
            return "subexpression";
        case symbol_type::string_literal:
            return "string literal";
        case symbol_type::character_literal:
            return "char literal";
        case symbol_type::documentation:
            return "docuemntation";
        case symbol_type::llvm_literal:
            return "llvm literal";
        case symbol_type::block:
            return "block";        
        case symbol_type::identifier:
            return "identifier";
        case symbol_type::newline:
            return "newline";
        case symbol_type::indent:
            return "indent";
        case symbol_type::abstraction_definition:
            return "abstraction definition";
        case symbol_type::variable_definition:
            return "variable definition";
    }
}
