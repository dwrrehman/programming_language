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

#include "llvm/IR/LLVMContext.h"

#include "preprocessor.hpp"
#include "compiler.hpp"
#include "interpreter.hpp"

#include <vector>


#define revert_and_return()    revert(saved); return {}

symbol parse_symbol(struct file file);
expression parse_expression(struct file file);
expression_list parse_expression_list(struct file file);
variable_symbol_list parse_variable_symbol_list(struct file file);

string_literal parse_string_literal(struct file file) {
    string_literal literal = {};
    auto saved = save();
    auto t = next();
    if (t.type != token_type::string) { revert_and_return(); }
    literal.literal = t;
    literal.error = false;
    return literal;
}

character_literal parse_character_literal(struct file file) {
    character_literal literal = {};
    auto saved = save();
    auto t = next();
    if (t.type != token_type::character) { revert_and_return(); }
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

documentation parse_documentation(struct file file) {
    documentation literal = {};
    auto saved = save();
    auto t = next();
    if (t.type != token_type::documentation) { revert_and_return(); }
    literal.literal = t;
    literal.error = false;
    return literal;
}

builtin parse_builtin(struct file file) {
    builtin literal = {};
    auto saved = save();
    auto t = next();
    if (t.type != token_type::builtin) { revert_and_return(); }
    literal.name = t;
    literal.error = false;
    return literal;
}

identifier parse_identifier(struct file file) {
    identifier literal = {};
    auto saved = save();
    auto t = next();
    if (t.type != token_type::identifier
     && t.type != token_type::operator_) { revert_and_return(); }
    literal.name = t;
    literal.error = false;
    return literal;
}


static bool is_close_paren(const token &t) {
    return t.type == token_type::operator_ && t.value == ")";
}

static bool is_open_paren(const token &t) {
    return t.type == token_type::operator_ && t.value == "(";
}

static bool is_colon(const token &t) {
    return t.type == token_type::operator_ && t.value == ":";
}

static bool is_open_brace(const token &t) {
    return t.type == token_type::operator_ && t.value == "{";
}

static bool is_close_brace(const token &t) {
    return t.type == token_type::operator_ && t.value == "}";
}

element parse_element(struct file file) {

    element element = {};

    auto saved = save();
    auto name = parse_symbol(file);

    if (name.error) {

        revert(saved);
        auto t = next();

        if (is_colon(t))
            element.is_colon = true;

        else { revert_and_return(); }
    }

    element.name = name;
    element.error = false;
    return element;
}

element_list parse_element_list(struct file file) {
    std::vector<element> elements = {};

    auto saved = save();
    auto element = parse_element(file);

    while (!element.error) {
        elements.push_back(element);
        saved = save();
        element = parse_element(file);
    }
    revert(saved);

    auto result = element_list {};
    result.elements = elements;
    result.error = false;
    return result;
}

function_signature parse_function_signature(struct file file) {

    function_signature signature = {};

    auto saved = save();
    auto t = next();
    if (!is_open_paren(t)) {
        revert_and_return();
    }

    auto call_signature = parse_element_list(file);
    if (call_signature.error) { revert_and_return(); }

    t = next();
    if (!is_close_paren(t)) {
        std::cout << "statement #6\n";
        print_parse_error(file.name, t.line, t.column, convert_token_type_representation(t.type), t.value, "\")\"");
        print_source_code(file.text, {t});
        revert_and_return();
    }

    saved = save();
    auto return_type = parse_expression(file);
    if (return_type.error) revert(saved);
    else {
        signature.has_return_type = true;
        signature.return_type = return_type;
    }

    saved = save();
    t = next();
    if (!is_colon(t)) {
        auto signature_type = parse_expression(file);
        if (signature_type.error) revert(saved);
        else {
            signature.has_signature_type = true;
            signature.signature_type = signature_type;
        }
    }

    signature.error = false;
    return signature;
}


variable_symbol parse_variable_symbol(struct file file) {

    variable_symbol s = {};
    auto saved = save();

    auto t = next();
    if (is_open_paren(t)) {
        auto subexpression = parse_expression(file);
        if (!subexpression.error) {
            t = next();
            if (is_close_paren(t)) {
                s.type = symbol_type::subexpression;
                s.subexpression = subexpression;
                s.error = false;
                return s;
            }
        }
    }
    revert(saved);

    auto llvm = parse_llvm_literal(file);
    if (!llvm.error) {
        s.type = symbol_type::llvm_literal;
        s.llvm = llvm;
        s.error = false;
        return s;
    }
    revert(saved);

    auto builtin = parse_builtin(file);
    if (!builtin.error) {
        s.type = symbol_type::builtin;
        s.builtin = builtin;
        s.error = false;
        return s;
    }
    revert(saved);

    auto identifier = parse_identifier(file);
    if (!identifier.error) {
        s.type = symbol_type::identifier;
        s.identifier = identifier;
        s.error = false;
        return s;
    }
    revert(saved);
    return {};
}


variable_symbol_list parse_variable_symbol_list(struct file file) {

    std::vector<variable_symbol> variable_symbols = {};

    auto saved = save();
    auto variable_symbol = parse_variable_symbol(file);
    while (!variable_symbol.error) {
        variable_symbols.push_back(variable_symbol);
        saved = save();
        variable_symbol = parse_variable_symbol(file);
    }
    revert(saved);

    if (variable_symbols.empty()) {
        revert_and_return();
    }

    auto result = variable_symbol_list {};
    result.symbols = variable_symbols;
    result.error = false;
    return result;
}

variable_signature parse_variable_signature(struct file file) {

    variable_signature signature = {};

    auto saved = save();
    auto variable_symbol_list = parse_variable_symbol_list(file);
    if (variable_symbol_list.error) { revert_and_return(); }
    signature.name = variable_symbol_list;

    auto t = next();
    if (!is_colon(t)) { revert_and_return(); }

    auto expression = parse_expression(file);
    if (expression.error) { revert_and_return(); }
    signature.signature_type = expression;
    signature.error = false;
    return signature;
}

block parse_block(struct file file) {

    block block = {};

    auto saved = save();
    auto t = next();
    if (!is_open_brace(t)) { revert_and_return(); }

    saved = save();
    t = next();
    if (!is_open_brace(t)) {

        block.is_open = true;
        revert(saved);

        auto expression_list = parse_expression_list(file);

        if (expression_list.error) {
            revert(saved);

            auto expression = parse_expression(file);
            if (expression.error) { revert_and_return(); }
            block.statements.expressions = {expression};

        } else block.statements = expression_list;
    } else {
        t = next();
        if (!is_close_brace(t)) {
            std::cout << "statement #4\n";
            print_parse_error(file.name, t.line, t.column, convert_token_type_representation(t.type), t.value, "\"}\"");
            print_source_code(file.text, {t});
            revert_and_return();
        }
        block.is_closed = true;
    }

    t = next();
    if (!is_close_brace(t)) {
        std::cout << "statement #3\n";
        print_parse_error(file.name, t.line, t.column, convert_token_type_representation(t.type), t.value, "\"}\"");
        print_source_code(file.text, {t});
        revert_and_return();
    }

    block.error = false;
    return block;
}

symbol parse_symbol(struct file file) {
    symbol s = {};
    auto saved = save();

    auto function_signature = parse_function_signature(file);
    if (!function_signature.error) {
        s.type = symbol_type::function_signature;
        s.function = function_signature;
        s.error = false;
        return s;
    }
    revert(saved);

    auto variable_signature = parse_variable_signature(file);
    if (!variable_signature.error) {
        s.type = symbol_type::variable_signature;
        s.variable = variable_signature;
        s.error = false;
        return s;
    }
    revert(saved);

    auto t = next();
    if (is_open_paren(t)) {
        auto subexpression = parse_expression(file);
        if (!subexpression.error) {
            t = next();
            if (is_close_paren(t)) {
                s.type = symbol_type::subexpression;
                s.subexpression = subexpression;
                s.error = false;
                return s;
            }
        }
    }
    revert(saved);

    auto string = parse_string_literal(file);
    if (!string.error) {
        s.type = symbol_type::string_literal;
        s.string = string;
        s.error = false;
        return s;
    }
    revert(saved);

    auto character = parse_character_literal(file);
    if (!character.error) {
        s.type = symbol_type::character_literal;
        s.character = character;
        s.error = false;
        return s;
    }
    revert(saved);

    auto documentation = parse_documentation(file);
    if (!documentation.error) {
        s.type = symbol_type::documentation;
        s.documentation = documentation;
        s.error = false;
        return s;
    }
    revert(saved);

    auto llvm = parse_llvm_literal(file);
    if (!llvm.error) {
        s.type = symbol_type::llvm_literal;
        s.llvm = llvm;
        s.error = false;
        return s;
    }
    revert(saved);

    auto block = parse_block(file);
    if (!block.error) {
        s.type = symbol_type::block;
        s.block = block;
        s.error = false;
        return s;
    }
    revert(saved);

    auto builtin = parse_builtin(file);
    if (!builtin.error) {
        s.type = symbol_type::builtin;
        s.builtin = builtin;
        s.error = false;
        return s;
    }
    revert(saved);

    auto identifier = parse_identifier(file);
    if (!identifier.error) {
        s.type = symbol_type::identifier;
        s.identifier = identifier;
        s.error = false;
        return s;
    }
    revert(saved);
    return s;
}

void newlines() {
    auto saved = save();
    auto newline = next();
    while (newline.type == token_type::operator_ && newline.value == "\n") {
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

expression parse_expression(struct file file) {

    std::vector<symbol> symbols = {};

    auto saved = save();
    auto symbol = parse_symbol(file);
    while (!symbol.error) {
        symbols.push_back(symbol);
        saved = save();
        symbol = parse_symbol(file);
    }
    revert(saved);

    if (symbols.empty()) {
        auto t = next();
        std::cout << "statement #2\n";
        print_parse_error(file.name, t.line, t.column, convert_token_type_representation(t.type), t.value, "expression");
        print_source_code(file.text, {t});
        revert(saved);
        return {};
    }

    auto result = expression {};
    result.symbols = symbols;
    result.error = false;
    return result;
}

expression parse_terminated_expression(struct file file) {

    auto indent_count = indents();
    auto expression = parse_expression(file);
    expression.indent_level = indent_count;

    auto t = next();

    if (t.type == token_type::null) return {};

    if (t.type != token_type::operator_ || t.value != "\n") {
        print_parse_error(file.name, t.line, t.column, convert_token_type_representation(t.type), t.value, "a newline");
        print_source_code(file.text, {t});
        return {};
    }
    expression.error = false;
    return expression;
}

expression_list parse_expression_list(struct file file) {

    std::vector<expression> expressions = {};

    newlines();

    auto saved = save();
    auto expression = parse_terminated_expression(file);
    while (!expression.error) {
        expressions.push_back(expression);
        saved = save();
        expression = parse_terminated_expression(file);
    }
    revert(saved);

    auto list = expression_list {};
    list.expressions = expressions;
    list.error = false;
    return list;
}

translation_unit parse_translation_unit(struct file file) {
    auto expression_list = parse_expression_list(file);
    translation_unit result {};
    result.list = expression_list;
    result.error = expression_list.error;
    return result;
}

translation_unit parse(struct preprocessed_file file, llvm::LLVMContext& context) {
    start_lex(file.unit);

    debug_token_stream();
    return {};

    auto unit = parse_translation_unit(file.unit);
    print_translation_unit(unit);
    return unit;
}
