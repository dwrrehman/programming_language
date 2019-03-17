//
//  lexer.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//
#include "lexer.hpp"
#include "lists.hpp"
#include "debug.hpp"
#include "error.hpp"

#include <vector>
#include <iostream>
#include <iomanip>
#include <unistd.h>


#define clear_and_return()  auto result = current; current = {}; return result;

enum class lexing_state {none, string, string_expression, identifier, number, documentation, character_or_llvm};

static std::string text = "";
static std::string filename = "";

static size_t c = 0;
static size_t line = 0;
static size_t column = 0;
static lexing_state state = lexing_state::none;
static struct token current = {};

// helpers:

bool is_operator(char c) {
    std::string s = "";
    s.push_back(c);
    for (auto op : operators) if (s == op) return true;
    return false;
}

bool is_identifierchar(char c) {
    return isalnum(c) || c == '_';
}

bool is_escape_sequence(std::string s) {
    const std::vector<std::string> sequences = {
        "\\n", "\\t", "\\r", "\\\\", "\\\"", "\\\'", "\\[", "\\b", "\\a",
    };
    for (auto seq : sequences) if (s == seq) return true;
    return false;
}

static void advance_by(size_t n) {
    for (size_t i = n; i--;) {
        if (text[c] == '\n') {
            c++; line++; column = 0;
        } else {
            c++; column++;
        }
    }
}

static bool isvalid(size_t c) { return c >= 0 && c < text.size(); }

static void set_current(enum token_type t, enum lexing_state s) {
    current.type = t;
    current.value = "";
    current.line = line;
    current.column = column;
    state = s;
}

static void check_for_lexing_errors() {
    if (state == lexing_state::string) print_lex_error(filename, "string", line, column);
    else if (state == lexing_state::documentation) print_lex_error(filename, "documentation", line, column);
    else if (state == lexing_state::character_or_llvm) print_lex_error(filename, "character or llvm", line, column);

    if (state == lexing_state::string || state == lexing_state::documentation || state == lexing_state::character_or_llvm) {
        print_source_code(text, {current});
        throw "lexing error";
    }
}

// the main lexing function:

struct token next() {
    while (true) {
        if (c >= text.size()) check_for_lexing_errors();
            
        // ------------------- starting and finising ----------------------

        else if (is_identifierchar(text[c]) && isvalid(c+1) && !is_identifierchar(text[c+1]) && state == lexing_state::none) {
            set_current(token_type::identifier, lexing_state::none);
            current.value = text[c];
            if (current.value == "_") current.type = token_type::keyword;
            advance_by(1);
            clear_and_return();

        // ---------------------- starting --------------------------

        } else if (text[c] == '\"' && state == lexing_state::none) { set_current(token_type::string, lexing_state::string);
        } else if (text[c] == '`' && state == lexing_state::none) { set_current(token_type::documentation, lexing_state::documentation);
        } else if (text[c] == '\'' && state == lexing_state::none) { set_current(token_type::llvm, lexing_state::character_or_llvm);
        } else if (is_identifierchar(text[c]) && state == lexing_state::none) {
            set_current(token_type::identifier, lexing_state::identifier);
            current.value += text[c];

        } else if (text[c] == '\\' && isvalid(c+1) && text[c+1] == ')' && state == lexing_state::none) {
            set_current(token_type::string, lexing_state::string);
            current.value.push_back((char)65);
            advance_by(1);

        // ---------------------- escaping --------------------------

        } else if (text[c] == '\\' && state == lexing_state::string) {
            if (isvalid(c+1) && text[c+1] == '\"') {
                current.value += "\"";
                advance_by(1);

            } else if (isvalid(c+1) && text[c+1] == 'n') {
                current.value += "\n";
                advance_by(1);

            } else if (isvalid(c+1) && text[c+1] == 't') {
                current.value += "\t";
                advance_by(1);

            } else if (isvalid(c+1) && text[c+1] == '(') {
                current.value.push_back((char)65);
                state = lexing_state::none;
                advance_by(2);
                clear_and_return();
            }

        //---------------------- finishing  ----------------------

        } else if ((text[c] == '\"' && state == lexing_state::string) ||
            (text[c] == '\'' && state == lexing_state::character_or_llvm) ||
            (text[c] == '`' && state == lexing_state::documentation)) {
            if (state == lexing_state::character_or_llvm && (current.value.size() == 1 || is_escape_sequence(current.value))) current.type = token_type::character;
            state = lexing_state::none;
            advance_by(1);
            clear_and_return();

        } else if (is_identifierchar(text[c]) && isvalid(c+1) && !is_identifierchar(text[c+1]) && state == lexing_state::identifier) {
            current.value += text[c];
            for (auto builtin : builtins) if (current.value == builtin) current.type = token_type::builtin;
            state = lexing_state::none;
            advance_by(1);
            clear_and_return();

        // ---------------- pushing ----------------

        } else if (state == lexing_state::string ||
                   state == lexing_state::character_or_llvm ||
                   state == lexing_state::documentation ||
                   (is_identifierchar(text[c]) && state == lexing_state::identifier)) {
            current.value += text[c];

        } else if (is_operator(text[c]) && state == lexing_state::none) {
            set_current(token_type::operator_, lexing_state::none);
            current.value = text[c];
            advance_by(1);
            clear_and_return();
        }
        advance_by(1);
    }
}

// this function should be called before lexing a given file.

void start_lex(const std::string given_filename, const std::string given_text) {
    text = given_text + "    ";
    filename = given_filename;
    c = 0;
    line = 1;
    column = 0;
    state = lexing_state::none;
    current = {};
}
