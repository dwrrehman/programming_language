//
//  preprocessor.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "preprocessor.hpp"
#include "lists.hpp"
#include "error.hpp"
#include "debug.hpp"
#include "compiler.hpp"

#include <unordered_map>
#include <iostream>
#include <vector>
#include <exception>

// structures:

enum class pptoken_type {none, text, open, close, seperator};

struct pp_token {
    enum pptoken_type type = pptoken_type::none;
    std::string text = "";
};

// Globals:

static size_t c = 0;
static size_t line = 0;
static size_t column = 0;
static std::string pp_text = "";
static std::string pp_filename = "";


// helpers:

static bool pp_is_valid(size_t c) {
    return c >= 0 && c < pp_text.size();
}

static bool next_token_is(std::string s, size_t c) {
    bool result = true;
//    for (int i = 0; i < s.size(); i++) {
//        if (is_valid(c+i))
//            result = result && (text[c+i] == s[i]);
//        else result = false;
//    }
    return result;
}

static void pp_advance_by(size_t n) {
    for (size_t i = n; i--;) {
        if (pp_text[c] == '\n') {
            c++; line++; column = 1;
        } else {
            c++; column++;
        }
    }
}

void start_pp_lex(const struct file file) {
    c = 0;
    pp_text = file.text;
    pp_filename = file.name;
    line = 1;
    column = 1;
}


std::vector<struct pp_token> pp_lex(struct file file) {

    std::vector<struct pp_token> tokens = {};

//    while (c < text.size()) {
//
//        if (next_token_is("::{", c)) {
//            advance_by(3);
//            tokens.push_back({pptoken_type::open, ""});
//
//        } else if (next_token_is(":::", c)) {
//            advance_by(3);
//            tokens.push_back({pptoken_type::seperator, ""});
//
//        } else if (next_token_is("::}", c)) {
//            advance_by(3);
//            tokens.push_back({pptoken_type::close, ""});
//
//        } else {
//            std::string s = "";
//            while (is_valid(c) && !next_token_is("::{", c) && !next_token_is(":::", c) && !next_token_is("::}", c)) {
//                s.push_back(text[c]);
//                advance_by(1);
//            }
//            tokens.push_back({pptoken_type::text, s});
//        }
//    }
    return tokens;
}


struct preprocessed_file pp_parse(std::vector<struct pp_token> tokens, struct file file) {

    return {file, {}};
}

struct preprocessed_file preprocess(struct file file) {
    start_pp_lex(file);
    auto tokens = pp_lex(file);
    return pp_parse(tokens, file);

}

/// this code is useful, but not right now:
//    for (auto macro : file.macros) {
//        auto pattern = interpret_llvm(frontend(file.unit, context, true));
//        auto body = interpret_llvm(frontend(file.unit, context, true));
//        macro_replace(pattern, body, file.unit.text);    //TODO: this is missing ast nodes in macros.
//    }



void macro_replace(std::string pattern, std::string body, std::string &text) {

}
