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

symbol parse_symbol(struct file file);
expression parse_expression(struct file file);


string_literal parse_string_literal(struct file file) {
    string_literal literal = {};
    auto saved = save();
    auto t = next();
    if (t.type == token_type::string) {
        literal.literal = t;
        literal.error = false;
        return literal;
    }
    revert(saved);
    return {};
}

character_literal parse_character_literal(struct file file) {
    character_literal literal = {};
    auto saved = save();
    auto t = next();
    if (t.type == token_type::character) {
        literal.literal = t;
        literal.error = false;
        return literal;
    }
    revert(saved);
    return {};
}

llvm_literal parse_llvm_literal(struct file file) {
    llvm_literal literal = {};
    auto saved = save();
    auto t = next();
    if (t.type == token_type::llvm) {
        literal.literal = t;
        literal.error = false;
        return literal;
    }
    revert(saved);
    return {};
}

documentation parse_documentation(struct file file) {
    documentation literal = {};
    auto saved = save();
    auto t = next();
    if (t.type == token_type::documentation) {
        literal.literal = t;
        literal.error = false;
        return literal;
    }
    revert(saved);
    return {};
}

builtin parse_builtin(struct file file) {
    builtin literal = {};
    auto saved = save();
    auto t = next();
    if (t.type == token_type::builtin) {
        literal.name = t;
        literal.error = false;
        return literal;
    }
    revert(saved);
    return {};
}

identifier parse_identifier(struct file file) {
    identifier literal = {};
    auto saved = save();
    auto t = next();
    if (t.type == token_type::identifier) {
        literal.name = t;
        literal.error = false;
        return literal;
    }
    revert(saved);
    return {};
}


element parse_element(struct file file) {
    element element = {};

    auto saved = save();
    auto name = parse_symbol(file);

    if (!name.error) {
        element.name = name;

        auto saved = save();
        auto t = next();

        if (t.type == token_type::operator_ && t.value == ":") {

            auto saved = save();
            auto expression = parse_expression(file);

            if (!expression.error) {
                element.has_type = true;
                element.type = expression;
                element.error = false;
                return element;
            }

            revert(saved);
            saved = save();
            auto t = next();

            print_parse_error(file.name, t.line, t.column,convert_token_type_representation(t.type), t.value, "an expression");
            print_source_code(file.text, {t});
            revert(saved);
            return {};
        }
        revert(saved);
        element.error = false;
        return element;
    }
    revert(saved);
    return {};
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

    auto saved = save();
    auto t = next();

    // check that that t is a "(". if it isnt, then simply move along, passing error up.

    saved = save();
    auto call_signature = parse_element_list(file);

    if (call_signature.error) {
        // simply leave, and pass error up. (done give error message)
    }

    saved = save();
    t = next();

    // if ( is not a ")") then
        /// given a error message, saying "expected ")" after call signature, found "{something here}".

    return {};
}

variable_signature parse_variable_signature(struct file file) {

    // tweak ebnf grammar so it isnt non turing decidable to parse. we need our variable name signature to be more simple!
    // note: you canot have literals (string, llvm, char, etc) in the name of a variable, though. this simplifies the grammar.

    return {};
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

    auto llvm = parse_llvm_literal(file);
    if (!llvm.error) {
        s.type = symbol_type::llvm_literal;
        s.llvm = llvm;
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

/*
 auto t = next();
 print_parse_error(filename, t.line, t.column, convert_token_type_representation(t.type), t.value, "symbol");
 print_source_code(text, {t});
 revert(saved);
 */

void newlines() {
    auto saved = save();
    auto newline = next();
    while (newline.type == token_type::operator_ && newline.value == "\n") {
        saved = save();
        newline = next();
    }
    revert(saved);
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
        // TODO: print error: expected expression.s
        return {};
    }

    auto result = expression {};
    result.symbols = symbols;
    result.error = false;
    return result;
}


expression parse_terminated_expression(struct file file) {
    auto expression = parse_expression(file);

    auto t = next();
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
    print_expression_list(expression_list);
    translation_unit result {};
    result.list = expression_list;
    result.error = expression_list.error;
    return result;
}

translation_unit parse(struct preprocessed_file file, llvm::LLVMContext& context) {

    for (auto macro : file.macros) {
        auto pattern = interpret_llvm(frontend(file.unit, context, true));
        auto body = interpret_llvm(frontend(file.unit, context, true));
        macro_replace(pattern, body, file.unit.text);
    }

    start_lex(file.unit);

    debug_token_stream();
    print_source_code(file.unit.text, {next()});

    return {};
    return parse_translation_unit(file.unit);
}
