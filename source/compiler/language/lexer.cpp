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

static bool is_operator(char c) {
    std::string s = "";
    s.push_back(c);
    for (auto op : operators) if (s == op) return true;
    if (!isascii(c)) return true;
    return false;
}

static bool is_identifierchar(char c) {
    return isalnum(c) || c == '_';
}

static bool isvalid(size_t c) {
    return c >= 0 && c < text.size();
}

static bool is_escape_sequence(std::string s) {
    const std::vector<std::string> sequences = {
        "\\n", "\\t", "\\r", "\\\\", "\\\"", "\\\'", "\\[", "\\b", "\\a",
    };
    for (auto seq : sequences) if (s == seq) return true;
    return false;
}

static void advance_by(size_t n) {
    for (size_t i = n; i--;) {
        if (text[c] == '\n') {
            c++; line++; column = 1;
        } else {
            c++; column++;
        }
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
    else if (state == lexing_state::documentation) print_lex_error(filename, "documentation", line, column);
    else if (state == lexing_state::character_or_llvm) print_lex_error(filename, "character or llvm", line, column);
    else if (state == lexing_state::multiline_comment) print_lex_error(filename, "multi-line comment", line, column);

    if (state == lexing_state::string ||
        state == lexing_state::documentation ||
        state == lexing_state::character_or_llvm ||
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

        if (text[c] == '\n' && state == lexing_state::none) {
            state = lexing_state::indent;
        }

        if (text[c] == ';' && isvalid(c+1) && isspace(text[c+1]) && (state == lexing_state::none || state == lexing_state::indent)) {
            state = lexing_state::comment;
        } else if (text[c] == ';' && !isspace(text[c+1]) && (state == lexing_state::none || state == lexing_state::indent)) {
            state = lexing_state::multiline_comment;

        // ------------------- starting and finising ----------------------

        } else if (is_identifierchar(text[c]) && isvalid(c+1) && !is_identifierchar(text[c+1]) && (state == lexing_state::none || state == lexing_state::indent)) {
            set_current(token_type::identifier, lexing_state::none);
            current.value = text[c];
            if (current.value == "_") current.type = token_type::keyword;
            advance_by(1);
            clear_and_return();

        // ---------------------- starting --------------------------

        } else if (text[c] == '\"' && (state == lexing_state::none || state == lexing_state::indent)) { set_current(token_type::string, lexing_state::string);
        } else if (text[c] == '`' && (state == lexing_state::none || state == lexing_state::indent)) { set_current(token_type::documentation, lexing_state::documentation);
        } else if (text[c] == '\'' && (state == lexing_state::none || state == lexing_state::indent)) { set_current(token_type::llvm, lexing_state::character_or_llvm);
        } else if (is_identifierchar(text[c]) && (state == lexing_state::none || state == lexing_state::indent)) {
            set_current(token_type::identifier, lexing_state::identifier);
            current.value += text[c];

        } else if (text[c] == '\\' && isvalid(c+1) && text[c+1] == ')' && state == lexing_state::none) {
            set_current(token_type::string, lexing_state::string);
            current.value.push_back((char)31);
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
                current.value.push_back((char)31);
                state = lexing_state::none;
                advance_by(2);
                clear_and_return();
            }

        //---------------------- finishing  ----------------------

        } else if ((text[c] == '\n' && state == lexing_state::comment) ||
                   (text[c] == ';' && state == lexing_state::multiline_comment)) {

            if (state == lexing_state::comment) {
                state = lexing_state::indent;
                current.type = token_type::operator_;
                current.value = "\n";
                advance_by(1);
                clear_and_return();
            }

            state = lexing_state::none;

        } else if ((text[c] == '\"' && state == lexing_state::string) ||
                   (text[c] == '\'' && state == lexing_state::character_or_llvm) ||
                   (text[c] == '`' && state == lexing_state::documentation)) {

            if (state == lexing_state::character_or_llvm &&
                (current.value.size() == 1 || is_escape_sequence(current.value)))
                current.type = token_type::character;
            state = lexing_state::none;
            advance_by(1);
            clear_and_return();

        } else if (is_identifierchar(text[c]) && isvalid(c+1) && !is_identifierchar(text[c+1])
                   && state == lexing_state::identifier) {

            current.value += text[c];

            for (auto builtin : builtins)
                if (current.value == builtin)
                    current.type = token_type::builtin;

            state = lexing_state::none;
            advance_by(1);
            clear_and_return();

        // ---------------- pushing ----------------

        } else if (state == lexing_state::string ||
                   state == lexing_state::character_or_llvm ||
                   state == lexing_state::documentation ||
                   (is_identifierchar(text[c]) && state == lexing_state::identifier)) {
            current.value += text[c];

        } else if (state == lexing_state::comment || state == lexing_state::multiline_comment) {
            // do nothing;

        } else if (is_operator(text[c]) && (state == lexing_state::none || state == lexing_state::indent)) {
            set_current(token_type::operator_, lexing_state::none);
            if (text[c] == '\n') state = lexing_state::indent;
            current.value = text[c];
            advance_by(1);
            clear_and_return();

        } else if (text[c] == ' ' && state == lexing_state::indent) {
            bool found_indent = true;
            for (int i = 0; i < spaces_count_for_indent; i++) {
                if (isvalid(c+i))
                    found_indent = found_indent && text[c+i] == ' ';
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

        } else if (text[c] == '\t' && state == lexing_state::indent) {
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
