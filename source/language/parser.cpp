//
//  parser.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "parser.hpp"
#include "nodes.hpp"
#include "lists.hpp"
#include "error.hpp"
#include "debug.hpp"

#define revert_and_return()    revert(saved); return {true, true, true}

/// ------------ prototypes -----------------

symbol parse_symbol(struct file file, bool newlines_are_a_symbol);
expression parse_expression(struct file file, bool can_be_empty, bool newlines_are_a_symbol);
expression_list parse_expression_list(struct file file, bool can_be_empty);
size_t indents(void);
void newlines(void);

inline static bool is_operator(token t) { return t.type == token_type::operator_; }
inline static bool is_identifier(token t) { return t.type == token_type::identifier; }
inline static bool is_close_paren(token t) { return is_operator(t) and t.value == ")"; }
inline static bool is_open_paren(token t) { return is_operator(t) and t.value == "("; }
inline static bool is_newline(token t) { return is_operator(t) and t.value == "\n"; }
inline static bool is_reserved_operator(token t) { return is_newline(t) or is_open_paren(t) or is_close_paren(t); }

/// ------------------ parsers ---------------------------

void newlines() {
    auto saved = save();
    auto t = next();
    while (is_newline(t)) {
        saved = save();
        t = next();
    }
    revert(saved);
}

size_t indents() {
    auto indent_count = 0;
    auto saved = save();
    auto indent = next();
    while (indent.type == token_type::indent) {
        indent_count++;
        saved = save();
        indent = next();
    }
    revert(saved);
    return indent_count;
}

string_literal parse_string_literal() {    
    auto saved = save();
    auto t = next();
    if (t.type != token_type::string) { revert_and_return(); }
    return {t};
}

llvm_literal parse_llvm_literal() {
    auto saved = save();
    auto t = next();
    if (t.type != token_type::llvm) { revert_and_return(); }
    return {t};
}

identifier parse_identifier() {    
    auto saved = save();
    auto t = next();
    if (not is_identifier(t) and (is_reserved_operator(t) 
                                  or not is_operator(t))) { revert_and_return(); }
    return {t};
}

symbol parse_symbol(file file, bool newlines_are_a_symbol) {    
    auto saved = save();    
    auto t = next();
    if (is_open_paren(t)) {
        auto expressions = parse_expression_list(file, true);
        if (not expressions.error) {
            auto saved_t = t;
            t = next();
            if (is_close_paren(t)) return { expressions };
            else {
                print_parse_error(file.name, saved_t.line, saved_t.column, convert_token_type_representation(t.type), t.value, "\")\" to close expression");
                print_source_code(file.text, {saved_t});
                revert_and_return();
            }
        }        
    } else revert(saved);

    auto string = parse_string_literal();
    if (not string.error) return string;
    else revert(saved);
    
    auto llvm = parse_llvm_literal();
    if (not llvm.error) return llvm;
    else revert(saved);

    auto identifier = parse_identifier(); 
    if (not identifier.error) return identifier;
    else revert(saved); 

    symbol s = {};
    t = next();
    if (is_newline(t) and newlines_are_a_symbol) {
        s.type = symbol_type::newline;
        return s;
    } else revert(saved);

    t = next();
    if (t.type == token_type::indent and newlines_are_a_symbol) {
        s.type = symbol_type::indent;
        return s;
    } revert_and_return();
}

expression parse_expression(file file, bool can_be_empty, bool newlines_are_a_symbol) {

    std::vector<symbol> symbols = {};
    auto saved = save();
    auto symbol = parse_symbol(file, newlines_are_a_symbol);
    while (not symbol.error) {
        symbols.push_back(symbol);
        saved = save();
        symbol = parse_symbol(file, newlines_are_a_symbol);
    }
    revert(saved);
    
    auto result = expression {symbols};
    result.error = not can_be_empty and symbols.empty();
    return result;
}

expression parse_terminated_expression(file file) {

    auto indent_count = indents();
    auto expression = parse_expression(file, /*can_be_empty = */true, /*newlines_are_a_symbol = */false);
    expression.indent_level = indent_count; 

    auto saved = save();
    auto t = next();
    if (not is_newline(t) and not (is_close_paren(t) and expression.symbols.size())) { revert_and_return(); }
    if (is_close_paren(t) and expression.symbols.size()) revert(saved);
    return expression;
}

expression_list parse_expression_list(file file, bool can_be_empty) {     
    newlines();
    
    auto saved = save();
    std::vector<expression> expressions = {};    
    auto expression = parse_terminated_expression(file);
    while (not expression.error) {
        expressions.push_back(expression);
        saved = save();
        expression = parse_terminated_expression(file);
    }
    revert(saved);
    
    return {expressions, expressions.empty() and not can_be_empty};
}

expression_list parse(file file) {
    start_lex(file);

    if (debug) {
        debug_token_stream();
        start_lex(file);
    }

    auto unit = parse_expression_list(file, /*can_be_empty = */true);    
    if (debug) print_translation_unit(unit, file);
    if (unit.error or next().type != token_type::null) throw "parse error:";
    else return unit;
}
