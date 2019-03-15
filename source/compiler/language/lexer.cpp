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

enum class lexing_state {none, string, string_expression, identifier, number, documentation, character, llvm};

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

static void advance_by(size_t n) {
    for (size_t i = n; i--;) {
        if (text[c] == '\n' && state == lexing_state::none) {
            c++; line++; column = 0;
        } else {
            c++; column++;
        }
    }
}

static bool isvalid(size_t c) { return c >= 0 && c < text.size(); }

struct token next() {
    if (text[c] == '\"' && state == lexing_state::none) {
        state = lexing_state::string;
        current.type = token_type::string;
        current.value = "";
        current.line = line;
        current.column = column;
        advance_by(1);
        
    } else if (text[c] == '\\' && state == lexing_state::string) {
        if (isvalid(c+1) && text[c+1] == '\"') {
            current.value += "\"";
            advance_by(2);
        } else if (isvalid(c+1) && text[c+1] == 'n') {
            current.value += "\n";
            advance_by(2);
        } else if (isvalid(c+1) && text[c+1] == 't') {
            current.value += "\t";
            advance_by(2);
        } else if (isvalid(c+1) && text[c+1] == '(') {
            advance_by(2);
            return current;
        }
        
    } else if (text[c] == '\"' && state == lexing_state::string) {
        
    }
    return {};
}

