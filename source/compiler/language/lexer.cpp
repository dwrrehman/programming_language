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

static std::string text = "";
static std::string filename = "";

static size_t c = 0;
static size_t line = 0;
static size_t column = 0;
static lexing_state state = lexing_state::indent;
static struct token current = {};

// helpers:
static bool is_identifier(char c) { return isalnum(c) or c == '_'; }
static bool is_operator(char c) { return (not is_identifier(c) or not isascii(c)) and c != ' ' and c != '\t'; }

static bool isvalid(size_t c) { return c >= 0 and c < text.size(); }

static void advance_by(size_t n) {
    for (size_t i = n; i--;) {
        if (text[c] == '\n') {
            c++; line++; column = 1;
        } else { c++; column++; }
    }
}

static void set_current(enum token_type t, enum lexing_state s) {
    current.type = t;
    current.value = "";
    current.line = line;
    current.column = column;
    state = s;
}

static void check_for_lexing_errors() {
    if (state == lexing_state::string) print_lex_error(filename, "string", line, column);
    else if (state == lexing_state::llvm_string) print_lex_error(filename, "llvm_string", line, column);
    else if (state == lexing_state::multiline_comment) print_lex_error(filename, "multi-line comment", line, column);

    if (state == lexing_state::string or
        state == lexing_state::llvm_string or
        state == lexing_state::multiline_comment) {
        print_source_code(text, {current});
        throw "lexing error";
    }
}

// the main lexing function:

struct token next() {
    while (true) {

        if (c >= text.size()) {
            check_for_lexing_errors();
            return {token_type::null, "", line, column};            
        }

        if (text[c] == '\n' and state == lexing_state::none) {
            state = lexing_state::indent;
        }

        if (text[c] == ';' and isvalid(c+1) and isspace(text[c+1]) and (state == lexing_state::none or state == lexing_state::indent)) {
            state = lexing_state::comment;
        } else if (text[c] == ';' and not isspace(text[c+1]) and (state == lexing_state::none or state == lexing_state::indent)) {
            state = lexing_state::multiline_comment;

        // ------------------- starting and finising ----------------------

        } else if (is_identifier(text[c]) and isvalid(c+1) and not is_identifier(text[c+1]) and (state == lexing_state::none or state == lexing_state::indent)) {
            set_current(token_type::identifier, lexing_state::none);
            current.value = text[c];
            advance_by(1);
            clear_and_return();

        // ---------------------- starting --------------------------

        } else if (text[c] == '\"' and (state == lexing_state::none or state == lexing_state::indent)) { set_current(token_type::string, lexing_state::string);
        } else if (text[c] == '`' and (state == lexing_state::none or state == lexing_state::indent)) { set_current(token_type::llvm, lexing_state::llvm_string);
        } else if (is_identifier(text[c]) and (state == lexing_state::none or state == lexing_state::indent)) {
            set_current(token_type::identifier, lexing_state::identifier);
            current.value += text[c];

        // ---------------------- escaping --------------------------

        } else if (text[c] == '\\' and state == lexing_state::string) {
            if (isvalid(c+1) and text[c+1] == '\"') {
                current.value += "\"";
                advance_by(1);

            } else if (isvalid(c+1) and text[c+1] == 'n') {
                current.value += "\n";
                advance_by(1);

            } else if (isvalid(c+1) and text[c+1] == 't') {
                current.value += "\t";
                advance_by(1);
            } 
        //---------------------- finishing  ----------------------

        } else if ((text[c] == '\n' and state == lexing_state::comment) or
                   (text[c] == ';' and state == lexing_state::multiline_comment)) {

            if (state == lexing_state::comment) {
                state = lexing_state::indent;
                current.type = token_type::operator_;
                current.value = "\n";
                advance_by(1);
                clear_and_return();
            }

            state = lexing_state::none;

        } else if ((text[c] == '\"' and state == lexing_state::string) or
                   (text[c] == '`' and state == lexing_state::llvm_string)) {
            state = lexing_state::none;
            advance_by(1);
            clear_and_return();

        } else if (is_identifier(text[c]) and isvalid(c+1) and !is_identifier(text[c+1]) and state == lexing_state::identifier) {
            current.value += text[c];
            state = lexing_state::none;
            advance_by(1);
            clear_and_return();

        // ---------------- pushing ----------------

        } else if (state == lexing_state::string or
                   state == lexing_state::llvm_string or
                   (is_identifier(text[c]) and state == lexing_state::identifier)) {
            current.value += text[c];

        } else if (state == lexing_state::comment or state == lexing_state::multiline_comment) {
            // do nothing

        } else if (is_operator(text[c]) and (state == lexing_state::none or state == lexing_state::indent)) {
            set_current(token_type::operator_, lexing_state::none);
            if (text[c] == '\n') state = lexing_state::indent;
            current.value = text[c];
            advance_by(1);
            clear_and_return();

        } else if (text[c] == ' ' and state == lexing_state::indent) {
            bool found_indent = true;
            for (int i = 0; i < spaces_count_for_indent; i++) {
                if (isvalid(c+i))
                    found_indent = found_indent and text[c+i] == ' ';
                else {
                    found_indent = false;
                    break;
                }
            }
            if (found_indent) {
                current.line = line;
                current.column = column;
                current.type = token_type::indent;
                current.value = " ";
                advance_by(spaces_count_for_indent);
                clear_and_return();
            }

        } else if (text[c] == '\t' and state == lexing_state::indent) {
            current.line = line;
            current.column = column;
            current.type = token_type::indent;
            current.value = " ";
            advance_by(1);
            clear_and_return();
        }
        advance_by(1);
    }
}

struct saved_state save() {
    struct saved_state result;
    result.saved_c = c;
    result.saved_line = line;
    result.saved_column = column;
    result.saved_state = state;
    result.saved_current = current;
    return result;
}

void revert(struct saved_state s) {
    c = s.saved_c;
    line = s.saved_line;
    column = s.saved_column;
    state = s.saved_state;
    current = s.saved_current;
}

// this function should be called before lexing a given file.
void start_lex(const struct file file) {
    text = file.text;
    filename = file.name;
    c = 0;
    line = 1;
    column = 1;
    state = lexing_state::indent;
    current = {};
}
