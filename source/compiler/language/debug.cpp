//
//  debug.cpp
//  language
//
//  Created by Daniel Rehman on 1903063.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "debug.hpp"

#include "arguments.hpp"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>


// ----------------------- command line arguments debugging: ----------------------------


void debug_arguments(struct arguments args) {
    std::cout << "file count = " <<  args.files.size() << "\n";
    for (auto a : args.files) {
        std::cout << "file: " << a.name << "\n";
        std::cout << "data: \n:::" << a.text << ":::\n";
    }
    
    std::cout << std::boolalpha;
    std::cout << "error = " << args.error << std::endl;
    std::cout << "use interpreter = " << args.use_interpreter << std::endl;
    std::cout << "exec name = " << args.executable_name << std::endl;    
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
        case token_type::indent: return "indent";
    }
}

void debug_token_stream() {
    std::vector<struct token> tokens = {};
    struct token t = {};
    while ((t = next()).type != token_type::null) tokens.push_back(t);
    print_lex(tokens);
}



// ---------------------------- parser debugging functions -------------------------------


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
    print_expression(abstraction.call_signature, d+2);

    prep(d+1); std::cout << "return type: \n";
    print_expression(abstraction.return_type, d+2);

    prep(d+1); std::cout << "abstraction body: \n";
    print_block(abstraction.body, d+2);
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
        default: break;
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
    prep(d); std::cout << "type = {\n";
    if (expression.type) {
        print_expression(*expression.type, d+1);
        prep(d); std::cout << "}\n";
    } else {
        prep(d+1); std::cout << "_type\n";
        prep(d); std::cout << "}\n";
    }
}


void print_abstraction_definition_line(abstraction_definition definition) {
    print_expression_line(definition.call_signature);
    std::cout << " -> ";
    print_expression_line(definition.return_type);
    std::cout << " ";
    print_block_line(definition.body);
}

void print_symbol_line(symbol symbol) {

    switch (symbol.type) {
        case symbol_type::identifier:
            std::cout << symbol.identifier.name.value;
            break;

        case symbol_type::llvm_literal:
            std::cout << "\'" << symbol.llvm.literal.value << "\'";
            break;

        case symbol_type::string_literal:
            std::cout << "\"" << symbol.string.literal.value << "\"";
            break;

        case symbol_type::subexpression:
            print_expression_line(symbol.subexpression);
            break;

        case symbol_type::block:
            print_block_line(symbol.block);
            break;

        case symbol_type::newline:
            std::cout << "{NEWLINE}";
            break;
        case symbol_type::indent:
            std::cout << "{INDENT}";
            break;

        case symbol_type::none:
            std::cout << "{NONE}\n";
            assert(false);
            break;
        case symbol_type::abstraction_definition:
            print_abstraction_definition_line(symbol.abstraction);
            break;
        default: break;
    }
}

void print_expression_line(expression expression) {
    std::cout << "(";
    int i = 0;
    for (auto symbol : expression.symbols) {
        print_symbol_line(symbol);
        if (i < expression.symbols.size() - 1) std::cout << " ";
        i++;
    }
    std::cout << ")";

    if (expression.type) {
        std::cout << ": (";
        print_expression_line(*expression.type);
        std::cout << ")";
    }
}








/// TODO: put this in analysis or some other important file. this is a avery improtant function.

std::string expression_to_string(expression given) {
    std::string result = "(";
    size_t i = 0;
    for (auto symbol : given.symbols) {
        if (symbol.type == symbol_type::identifier) result += symbol.identifier.name.value;
        else if (symbol.type == symbol_type::subexpression) {
            result += "(" + expression_to_string(symbol.subexpression) + ")";
        }
        if (i < given.symbols.size() - 1) result += " ";
        i++;
    }
    result += ")";
    if (given.llvm_type) {
        std::string type = "";
        given.llvm_type->print(llvm::raw_string_ostream(type) << "");
        result += " " + type;
    } else if (given.type) {
        result += " " + expression_to_string(*given.type);
    }
    return result;
}










void print_block_line(block block) {
    std::cout << "{\n";
    print_expression_list(block.list, 1);
    std::cout << "}";
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
    }
}


void print_stack(std::vector<std::vector<expression>> stack) {
    std::cout << "\n\n---------------- printing stack -------------------\n\n";
    for (int i = 0; i < stack.size(); i++) {
        std::cout << "----- FRAME # " << i << " -------------------\n";
        for (int j = 0; j < stack[i].size(); j++) {
            std::cout << "#" << j << " : ";
            
            //print_expression_line(stack[i][j]);    /// Bad.
            
            auto s = expression_to_string(stack[i][j]);
            std::cout << s << "\n";             
        }
        std::cout << "---------------------------------------------\n\n";
    }
    std::cout << "\n\n-----------------------------------------------------------\n\n";
}
