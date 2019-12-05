//
//  debug.cpp
//  language
//
//  Created by Daniel Rehman on 1903063.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "debug.hpp"

#include "arguments.hpp"
#include "builtins.hpp"
#include "symbol_table.hpp"

#include <iostream>


// ----------------------- command line arguments debugging: ----------------------------


void debug_arguments(const arguments& args) {
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


void print_lex(const std::vector<struct token>& tokens) {
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


void print_symbol(symbol s, int d);
void print_expression(expression s, int d);

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
            prep(d); std::cout << "list symbol\n";
            print_expression_list(symbol.expressions, d+1);
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
    
    prep(d); std::cout << "type = " << expression.type << "\n";
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
    if (expression.type)
        std::cout << ": " << stringify_intrin(expression.type);
}

void print_expression_list(expression_list list, int d) {

    prep(d); std::cout << "expression list:\n";
    prep(d); std::cout << "expression count = " << list.list.size() << "\n";
    int i = 0;
    for (auto e : list.list) {
        prep(d+1); std::cout << "expression #" << i << "\n";
        prep(d+1); std::cout << std::boolalpha << "error: " << e.error << "\n";
        print_expression(e, d+2);
        prep(d+1); std::cout << "\n";
        i++;
    }
}

void print_expression_list_line(expression_list list) {
    std::cout << "{\n";
    int i = 0;
    for (auto e : list.list) {
        std::cout << "\t" << i << ": ";
        print_expression_line(e);        
        std::cout << "\n";
        i++;
    }
    std::cout << "}\n";
}

void print_translation_unit(expression_list unit, file file) { 
    std::cout << "translation unit: (" << file.name << ")\n";
    print_expression_list(unit, 1);
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
        case symbol_type::identifier:
            return "identifier";
        case symbol_type::newline:
            return "newline";
        case symbol_type::indent:
            return "indent";
    }
}


////void debug_table(const std::unique_ptr<llvm::Module>& module, symbol_table& stack) {
//////    std::cout << "NOTE: updating stack...\n";
//////    stack.update(module->getValueSymbolTable()); 
//////    
//////    std::cout << "print_stack: \n"; 
//////    print_stack(stack);
//////    
//////    std::cout << "print_master: \n";
//////    print_master(stack);
//////    
//////    std::cout << "print_index_top_stack: \n";
//////    print_index_top_stack(stack);
//////    
//////    std::cout << "print_simply_master: \n";
//////    print_simply_master(stack);
//////    
//////    std::cout << "print_llvm_symtable: \n";
//////    print_llvm_symtable(module->getValueSymbolTable());
//////        
////    
////    stack.debug();
////    std::cout << "\n\n\n\n";
//}





void print_resolved_list(resolved_expression_list list, nat depth, state& state);
void print_resolved_expr(resolved_expression expr, nat depth, state& state);

void print_resolved_expr(resolved_expression expr, nat depth, state& state) {
    prep(depth); std::cout << "[error = " << std::boolalpha << expr.error << "]\n";
    prep(depth);
    
    std::cout 
    << "index = " 
    << expr.index << " :: " 
    << expression_to_string(state.stack.get(expr.index), state.stack);
    
    if (expr.signature.symbols.size()) {
        std::cout << " ::: " << expression_to_string(expr.signature, state.stack);
    }
    
    std::cout << "\n";    
    
    if (expr.llvm_type) { prep(depth); std::cout << "llvm type = "; expr.llvm_type->print(llvm::errs()); }
    std::cout << "\n";
    nat i = 0;
    for (auto arg : expr.args) {
        prep(depth + 1); std::cout << "argument #" << i++ << ": \n";
        print_resolved_list(arg, depth + 2, state);
        prep(depth); std::cout << "\n";
    }
}

void print_resolved_list(resolved_expression_list list, nat depth, state& state) {
    prep(depth); std::cout << "[error = " << std::boolalpha << list.error << "]\n";
    nat i = 0;
    for (auto expr : list.list) {        
        prep(depth + 1); std::cout << "expression #" << i++ << ": \n"; 
        print_resolved_expr(expr, depth + 2, state);
        prep(depth); std::cout << "\n";
    }
}

void print_resolved_unit(resolved_expression_list unit, state& state) {
    std::cout << "---------- printing resolved tranlation unit: ------------\n\n";
    print_resolved_list(unit, 0, state);
}

void print_nat_vector(std::vector<nat> v, bool newline) {
    std::cout << "[ ";
    for (auto e : v) {
        std::cout << e << " ";
    }
    std::cout << "]";
    if (newline) std::cout << "\n";
}
