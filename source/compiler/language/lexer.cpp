//
//  lexer.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//
#include "lexer.hpp"
#include "lists.hpp"

#include <vector>
#include <iostream>
#include <iomanip>


void print_lex(const std::vector<struct token> &tokens) {
    std::cout << "::::::::::LEX:::::::::::" << std::endl;
    for (auto token : tokens) {
        std::cout << "TOKEN(type: " << convert_token_type_representation(token.type) << ", value: \"" << (token.value != "\n" ? token.value : "\\n") << "\", [" << token.line << ":" << token.column << "])" << std::endl;
    }
    std::cout << ":::::::END OF LEX:::::::" << std::endl;
}

const char* convert_token_type_representation(enum token_type type) {
    switch (type) {
        case null_type: return "{null}";
        case string_type: return "string";
        case identifier_type: return "identifier";
        case number_type: return "number";
        case keyword_type: return "keyword";
        case operator_type: return "operator";
        case documentation_type: return "documentation";
        case character_type: return "character_or_llvm";
    }
}

bool is_leading_identifier_char(char c) {return isalpha(c) || c == '_';}
bool is_leading_number_char(char first, char second) {return isnumber(first) || (first == '.' && isnumber(second));;}

bool is_identifier_char(char c) {return isalpha(c) || c == '_' || isnumber(c);}
bool is_number_char(char c) {return isnumber(c) || c == '.';}

bool is_delimiting_number_char(char first, char second) {return is_number_char(first) && !is_number_char(second);}
bool is_delimiting_identifier_char(char first, char second) {return is_identifier_char(first) && !is_identifier_char(second);}

bool is_leading_delimiting_number_char(char first, char second) {return is_leading_number_char(first, second) && !is_number_char(second);}
bool is_leading_delimiting_identifier_char(char first, char second) {return is_leading_identifier_char(first) && !is_identifier_char(second);}

bool all_are_false_other_than(enum lexing_state desired, std::vector<bool> states) {
    for (int i = 0; i < states.size(); i++) {
        if (states[i] && i != desired) {
            return false;
        }
    }
    return true;
}

bool all_are_false(std::vector<bool> states) {
    for (auto s : states) if (s) return false;
    return true;
}

static void append_and_delimit(bool append, char c, size_t column, bool delimiting_mode, enum lexing_state desired, size_t line, std::vector<bool> &states, std::vector<std::string> &strings, std::vector<struct token> &tokens, enum token_type type) {
    if (append) strings[desired].push_back(c);
    if (delimiting_mode) {
        tokens.push_back({strings[desired], type, line, column});
        strings[desired] = "";
        states[desired] = false;
    }
}

bool recognize_state(
     enum lexing_state desired,
     char c,
     bool leading_mode,
     bool condition,
     std::vector<bool> &states,
     std::vector<std::string> &strings,
     bool append,
     std::vector<struct token> &tokens,
     bool delimiting_mode = false,
     enum token_type type = null_type,
     size_t line = 0, size_t column = 0
     ) {
    
    if (leading_mode && all_are_false(states) && condition) {
        states[desired] = true;
        append_and_delimit(append, c, column, delimiting_mode, desired, line, states, strings, tokens, type);
        return true;
    } else if (!leading_mode && states[desired] && all_are_false_other_than(desired, states) && condition) {
        append_and_delimit(append, c, column, delimiting_mode, desired, line, states, strings, tokens, type);
        return true;
    } else return false;
}

bool recognize_character_sequence(std::string text, int &start, std::vector<std::string> sequences, enum token_type type, size_t line, size_t &column, std::vector<struct token> &tokens, bool delimit_natrually) {
    for (auto sequence : sequences) {
        for (int c = 0; c < sequence.size(); c++)
            if (text[start + c] != sequence[c]) goto skip;

        if ((delimit_natrually && (sequence == "." || sequence == "-") && !is_number_char(text[start + sequence.size()])) ||
            (delimit_natrually && (sequence != "." && sequence != "-")) ||
            (!delimit_natrually && !is_identifier_char(text[start + sequence.size()]))) {
            
            tokens.push_back({sequence, type, line, column});
            start += sequence.size() - 1;
            column += sequence.size() - 1;
            return true;
        }
    skip: continue;
    }
    return false;
}


std::vector<struct token> lex(std::string text, bool &error) {
        
    sort_lists_by_decreasing_length();
    const size_t text_length = text.size();
    for (int i = 0; i < keywords[0].size() + 1; i++) text.push_back(' ');
    
    std::vector<struct token> tokens = {};
    std::vector<bool> states = {false, false, false, false, false};
    std::vector<std::string> strings = {"", "", "", "", ""};
    size_t line = 1, column = 0;
    size_t found_line = 1, found_column = 0;
    
    for (int c = 0; c < text_length; c++) {
        column++;
        const char first_char = text[c];
        const char second_char = text[c + 1];
            
        if (all_are_false(states) && recognize_character_sequence(text, c, keywords, keyword_type, line, column, tokens, false)) {
        } else if (all_are_false(states) && recognize_character_sequence(text, c, operators, operator_type, line, column, tokens, true)) {
            
            // leading delimiting:
        } else if (recognize_state(number_state, first_char, true, is_leading_delimiting_number_char(first_char, second_char), states, strings, true, tokens, true, number_type, line, column)) {
        } else if (recognize_state(identifier_state, first_char, true, is_leading_delimiting_identifier_char(first_char, second_char), states, strings, true, tokens, true, identifier_type, line, column)) {
            
            // delimiting:
        } else if (recognize_state(number_state, first_char, false, is_delimiting_number_char(first_char, second_char), states, strings, true, tokens, true, number_type, found_line, found_column)) {
        } else if (recognize_state(identifier_state, first_char, false, is_delimiting_identifier_char(first_char, second_char), states, strings, true, tokens, true, identifier_type, found_line, found_column)) {
        } else if (recognize_state(string_state, first_char, false, first_char == '\"', states, strings, false, tokens, true, string_type, found_line, found_column)) {
        } else if (recognize_state(documentation_state, first_char, false, first_char == '`', states, strings, false, tokens, true, documentation_type, found_line, found_column)) {
        } else if (recognize_state(character_state, first_char, false, first_char == '\'', states, strings, false, tokens, true, character_type, found_line, found_column)) {
            
            // leading:
        } else if (recognize_state(number_state, first_char, true, is_leading_number_char(first_char, second_char), states, strings, true, tokens)) {
            found_line = line;
            found_column = column;
        } else if (recognize_state(identifier_state, first_char, true, is_leading_identifier_char(first_char), states, strings, true, tokens)) {
            found_line = line;
            found_column = column;
        } else if (recognize_state(string_state, first_char, true, first_char == '\"', states, strings, false, tokens)) {
            found_line = line;
            found_column = column;
        } else if (recognize_state(documentation_state, first_char, true, first_char == '`', states, strings, false, tokens)) {
            found_line = line;
            found_column = column;
        } else if (recognize_state(character_state, first_char, true, first_char == '\'', states, strings, false, tokens)) {
            found_line = line;
            found_column = column;
            
            // neither:
        } else if (recognize_state(number_state, first_char, false, is_number_char(first_char), states, strings, true, tokens)) {
        } else if (recognize_state(identifier_state, first_char, false, is_identifier_char(first_char), states, strings, true, tokens)) {
        } else if (recognize_state(string_state, first_char, false, true, states, strings, true, tokens)) {
        } else if (recognize_state(documentation_state, first_char, false, true, states, strings, true, tokens)) {
        } else if (recognize_state(character_state, first_char, false, true, states, strings, true, tokens)) {
            
        } else if (first_char == ' ' || first_char == '\t') {
        } else {
            ///TODO: add a call to print_lex_error(first_char,line, column),     and make sure to print the source code!
            std::cout << "Error: Unexpected \"" << first_char << "\" at line " << line << ", column " << column << "." << std::endl;
            error = true;
        }
        
        if (first_char == '\n') {
            line++;
            column = 0;
        }
    }
    tokens.push_back({"\n", operator_type, line++, 0});
    
    print_lex(tokens); /// debug
    
    return tokens;
}
