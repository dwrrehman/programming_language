//
//  parser.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "parser.hpp"
#include "lexer.hpp"
#include "nodes.hpp"
#include "lists.hpp"
#include "error.hpp"
#include "debug.hpp"
#include "arguments.hpp"
#include "compiler.hpp"
#include "interpreter.hpp"

#include "llvm/IR/LLVMContext.h"

#include <vector>
#include <iostream>

#define revert_and_return()    revert(saved); return {}

/// ------------ prototypes -----------------

symbol parse_symbol(struct file file, bool newlines_are_a_symbol);
expression parse_expression(struct file file, bool can_be_empty, bool newlines_are_a_symbol);
expression_list parse_expression_list(struct file file, bool can_be_empty);
size_t indents(void);
void newlines(void);

/// ------------------ literal parsers ---------------------------

string_literal parse_string_literal(struct file file) {
    string_literal literal = {};
    auto saved = save();
    auto t = next();
    if (t.type != token_type::string) { revert_and_return(); }
    literal.literal = t;
    literal.error = false;
    return literal;
}

llvm_literal parse_llvm_literal(struct file file) {
    llvm_literal literal = {};
    auto saved = save();
    auto t = next();
    if (t.type != token_type::llvm) { revert_and_return(); }
    literal.literal = t;
    literal.error = false;
    return literal;
}

bool is_syntax(std::string value) {
    return std::find(language_syntax.begin(), language_syntax.end(), value) != language_syntax.end();
}

identifier parse_identifier(struct file file) {
    identifier literal = {};
    auto saved = save();
    auto t = next();
    if (t.type != token_type::identifier
     && (t.type != token_type::operator_ or t.value == "\n" or is_syntax(t.value))) {
        revert_and_return();
    }
    literal.name = t;
    literal.error = false;
    return literal;
}


// ------------------ token comparisons ---------------------------

static bool is_close_paren(const token &t) {
    return t.type == token_type::operator_ and t.value == ")";
}

static bool is_open_paren(const token &t) {
    return t.type == token_type::operator_ and t.value == "(";
}

static bool is_open_brace(const token &t) {
    return t.type == token_type::operator_ and t.value == "{";
}

static bool is_close_brace(const token &t) {
    return t.type == token_type::operator_ and t.value == "}";
}

// -------------------- ebnf parsers --------------------------------

block parse_block(struct file file) {

    block block = {};

    auto saved = save();
    auto t = next();
    auto st = t;
    if (not is_open_brace(t)) { revert_and_return(); }

    newlines();
    
    saved = save();

    auto expression_list = parse_expression_list(file, /*can_be_empty = */true);
    if (expression_list.error or !expression_list.expressions.size()) { // then we know that it must only be a single expression with no newline at the end.
        revert(saved);
        auto expression = parse_expression(file, /*can_be_empty = */true, /*newlines_are_a_symbol = */false);
        if (expression.error) { revert_and_return(); }
        block.list.expressions = {expression};
    } else block.list = expression_list;

    indents();
    t = next();
    if (not is_close_brace(t)) {
        print_parse_error(file.name, st.line, st.column, convert_token_type_representation(t.type), t.value, "\"}\" to close block");
        print_source_code(file.text, {st});
        revert_and_return();
    }

    block.error = false;
    return block;
}

symbol parse_symbol(struct file file, bool newlines_are_a_symbol) {
    symbol s = {};
    auto saved = save();

    auto t = next();
    if (is_open_paren(t)) {
        auto subexpression = parse_expression(file, /*can_be_empty = */true, /*newlines_are_a_symbol = */true);
        if (!subexpression.error) {
            auto st = t;
            t = next();
            if (is_close_paren(t)) {
                s.type = symbol_type::subexpression;
                s.subexpression = subexpression;
                s.error = false;
                return s;
            } else {
                print_parse_error(file.name, st.line, st.column, convert_token_type_representation(t.type), t.value, "\")\" to close subexpression");
                print_source_code(file.text, {st});
                revert_and_return();
            }
        }
    }
    revert(saved);

    auto string = parse_string_literal(file);
    if (not string.error) {
        s.type = symbol_type::string_literal;
        s.string = string;
        s.error = false;
        return s;
    }
    revert(saved);
    
    auto llvm = parse_llvm_literal(file);
    if (not llvm.error) {
        s.type = symbol_type::llvm_literal;
        s.llvm = llvm;
        s.error = false;
        return s;
    }
    revert(saved);

    auto block = parse_block(file);
    if (not block.error) {
        s.type = symbol_type::block;
        s.block = block;
        s.error = false;
        return s;
    }
    revert(saved);

    auto identifier = parse_identifier(file);
    if (not identifier.error) {
        s.type = symbol_type::identifier;
        s.identifier = identifier;
        s.error = false;
        return s;
    }
    revert(saved);

    t = next();
    if (t.type == token_type::operator_ and t.value == "\n" and newlines_are_a_symbol) {
        s.type = symbol_type::newline;
        s.error = false;
        return s;
    }
    revert(saved);

    t = next();
    if (t.type == token_type::indent and newlines_are_a_symbol) {
        s.type = symbol_type::indent;
        s.error = false;
        return s;
    }

    revert(saved);
    return s;
}

void newlines() {
    auto saved = save();
    auto newline = next();
    while (newline.type == token_type::operator_ and newline.value == "\n") {
        saved = save();
        newline = next();
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

expression parse_expression(struct file file, bool can_be_empty, bool newlines_are_a_symbol) {

    std::vector<symbol> symbols = {};

    auto saved = save();
    auto symbol = parse_symbol(file, newlines_are_a_symbol);
    while (not symbol.error) {
        symbols.push_back(symbol);
        saved = save();
        symbol = parse_symbol(file, newlines_are_a_symbol);
    }
    revert(saved);

    auto result = expression {};
    result.symbols = symbols;
    if (symbols.size() or can_be_empty) result.error = false;
    return result;
}

expression parse_terminated_expression(struct file file) {

    auto indent_count = indents();
    auto expression = parse_expression(file, /*can_be_empty = */true, /*newlines_are_a_symbol = */false);
    expression.indent_level = indent_count; 

    auto saved = save();
    auto t = next();

    if (t.type != token_type::operator_ or t.value != "\n") { revert_and_return(); }
    expression.error = false;
    return expression;
}

expression_list parse_expression_list(struct file file, bool can_be_empty) {

    std::vector<expression> expressions = {};

    newlines();

    auto saved = save();
    auto expression = parse_terminated_expression(file);
    while (not expression.error) {
        expressions.push_back(expression);
        saved = save();
        expression = parse_terminated_expression(file);
    }
    revert(saved);

    auto list = expression_list {};
    list.expressions = expressions;
    if (expressions.size() or can_be_empty) {
        list.error = false;
    }
    return list;
}

translation_unit parse_translation_unit(struct file file) {
    auto expression_list = parse_expression_list(file, /*can_be_empty = */true);
    translation_unit result {};
    result.list = expression_list;
    result.error = expression_list.error;
    return result;
}

translation_unit parse(struct file file, llvm::LLVMContext& context) {
    start_lex(file);

    if (debug) {
        debug_token_stream();
        start_lex(file);
        std::cout << "\n\n\n";
    }

    translation_unit unit = parse_translation_unit(file);
    if (debug) print_translation_unit(unit, file);
    if (unit.error or next().type != token_type::null) throw "parse error:";
    else return unit;
}
