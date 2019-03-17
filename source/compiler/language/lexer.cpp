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

void start_lex(const std::string given_filename, const std::string given_text) {
    text = given_text + "    ";
    filename = given_filename;
    c = 0;
    line = 1;
    column = 0;
    state = lexing_state::none;
    current = {};
}

bool is_operator(char c) {
    std::string s = "";
    s.push_back(c);
    for (auto op : operators)
        if (s == op) return true;
    return false;
}

bool is_identifierchar(char c) {
    return isalnum(c) || c == '_';
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

struct token next() {

    while (true) {

        std::cout << "c = " << c << "\n";
        std::cout << "text[c] = " << "\"" << text[c] << "\"\n";

        if (c >= text.size()) { return {};

        // ------------------- starting and finising ----------------------

        } else if (is_identifierchar(text[c]) && isvalid(c+1) && !is_identifierchar(text[c+1]) && state == lexing_state::none) {
            set_current(token_type::identifier, lexing_state::none);
            current.value = text[c];
            if (current.value == "_") current.type = token_type::keyword;
            std::cout << "finsihed single identifier\n\n";
            advance_by(1);
            clear_and_return();

        // ---------------------- starting --------------------------

        } else if (text[c] == '\"' && state == lexing_state::none) { set_current(token_type::string, lexing_state::string);
            std::cout << "string start\n\n";
        } else if (text[c] == '`' && state == lexing_state::none) { set_current(token_type::documentation, lexing_state::documentation);
            std::cout << "doc start\n\n";
        } else if (text[c] == '\'' && state == lexing_state::none) { set_current(token_type::character_or_llvm, lexing_state::character_or_llvm);
            std::cout << "char start\n\n";
        } else if (is_identifierchar(text[c]) && state == lexing_state::none) {
            set_current(token_type::identifier, lexing_state::identifier);
            current.value += text[c];
            std::cout << "identifier start\n\n";

        // ---------------------- escaping/finishing --------------------------

        } else if (text[c] == '\\' && state == lexing_state::string) { // any others, we may have to use the llvm trick, and simply have the user put the ascii code the character they want.
            std::cout << "found delimiter\n\n";
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
                advance_by(2);
                state = lexing_state::none;
                std::cout << "escaped string\n\n";
                clear_and_return();
            }

        } else if (text[c] == '\\' && isvalid(c+1) && text[c+1] == ')' && state == lexing_state::none) {
            set_current(token_type::string, lexing_state::string);
            current.value.push_back((char)65);
            advance_by(1);
            std::cout << "unescaped string\n\n";

        } else if (text[c] == '\"' && state == lexing_state::string) {
            advance_by(1);
            state = lexing_state::none;
            std::cout << "finsihed string\n\n";
            clear_and_return();

        } else if (is_identifierchar(text[c]) && isvalid(c+1) && !is_identifierchar(text[c+1]) && state == lexing_state::identifier) {
            current.value += text[c];
            advance_by(1);
            state = lexing_state::none;
            std::cout << "finsihed identifier\n\n";
            clear_and_return();


        // ---------------- pushing ----------------

        } else if (state == lexing_state::string) {
            std::cout << "pushing string\n\n";
            current.value += text[c];

        } else if (is_identifierchar(text[c]) && state == lexing_state::identifier) {
            std::cout << "pushing identifier\n\n";
            current.value += text[c];

        } else if (state == lexing_state::documentation) {
            std::cout << "pushing doc\n\n";
            current.value += text[c];

        } else if (is_operator(text[c]) && state == lexing_state::none) {
            set_current(token_type::operator_, lexing_state::none);
            current.value = text[c];
            advance_by(1);
            std::cout << "pushing op\n\n";
            clear_and_return();
        }
        //sleep(1);
        advance_by(1);
    }
}

