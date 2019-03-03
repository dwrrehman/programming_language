//
//  preprocessor.cpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "preprocessor.hpp"

#include <iostream>
#include <vector>





// --------------- preprocessor structures ----------------------------

std::vector<std::string> pp_keywords = {
    "\\",
    "replace", "begin",
    "with", "end",
    "if", "while",
    "use", "let", "be", "==", "<", ">"
    "++", "--", "i32", "not", "and", "or",
    "+", "-", "/", "*",
    "(", ")",
    "call", "function", "does",
};

enum pp_token_type {
    
    pp_keyword_type,
    pp_text_type,
    pp_identifier_type,
    pp_ast_node_type
};

struct pp_token {
    enum pp_token_type type;
    std::string value;
    size_t line;
    size_t column;
};


// helpers:

const char* convert_pp_token_type_representation(enum pp_token_type type) {
    switch (type) {
        case pp_text_type: return "text";
        case pp_identifier_type: return "identifier";
        case pp_keyword_type: return "keyword";
        case pp_ast_node_type: return "astnode";
    }
}

void print_pp_lex(const std::vector<struct pp_token> &tokens) {
    std::cout << "::::::::::PP LEX:::::::::::" << std::endl;
    for (auto token : tokens) {
        std::cout << "TOKEN(type: " << convert_pp_token_type_representation(token.type) << ", value: \"" << (token.value != "\n" ? token.value : "\\n") << "\", [" << token.line << ":" << token.column << "])" << std::endl;
    }
    std::cout << ":::::::END OF PP LEX:::::::\n\n\n" << std::endl;
}



// ----------------------------- pre-preprocessor -----------------------------------

std::string strip_comments(std::string text, bool &error) {
    
    text.append("    ");
    
    std::cout << "orginal text:\n::::" << text << ":::\n";
    
    std::string result = "";
    bool in_multi_comment = false;
    bool in_line_comment = false;
    
    for (int c = 0; c < text.size(); c++) {
        if (!in_line_comment && !in_multi_comment && text[c] == ';' && text[c+1] == ';') {
            in_line_comment = true;
            result.push_back(' ');
            
        } else if (in_line_comment && !in_multi_comment && text[c] == '\n') {
            result.push_back('\n');
            in_line_comment = false;
            
        } else if (!in_line_comment && !in_multi_comment && text[c] == ';' && text[c+1] == ':') {
            in_multi_comment = true;
            result.push_back(' ');
            
        } else if (!in_line_comment && in_multi_comment && text[c-1] == ':' && text[c] == ';') {
            in_multi_comment = false;
            result.push_back(' ');
            
        } else if (text[c] == '\n') {
            result.push_back('\n');
            
        } else if (text[c] == '\t') {
            result.push_back(' ');
        
        } else if (!in_line_comment && !in_multi_comment) {
            result.push_back(text[c]);
    
        } else {
            result.push_back(' ');
        }
    }
    
    if (in_multi_comment) {
        std::cout << "Error: unterminated multiline comment." << std::endl;
        ///TODO: add call using the standard error printing class.
        error = true;
    }
    
    std::cout << "new text:\n::::" << result << ":::\n";
    
    return result;
}










// --------------------------- lexer ------------------------------



std::vector<struct pp_token> pp_lexer(std::string text, bool &error) {
    
    std::vector<struct pp_token> tokens = {};
    
    bool inside_text = true;  // we start out assuming we are in the text, untill we encounter a keyword/identifier.
    bool inside_identifier = false;
    bool inside_ast_node = false;
    bool inside_keyword = false;
    
    std::string current_identifier = "";
    std::string current_text = "";
    std::string current_ast_node = "";
    std::string current_keyword = "";
    
    size_t line = 1, column = 0;

    size_t current_line = line;
    size_t current_column = column;
    
    text.append(" ");
    
    for (int c = 0; c < text.size() - 1; c++) {
        column++;

        if (text[c] == '\\' && inside_text) {   // found a astnode, identifier, or a keyword:
            
            if (text[c+1] == '\\' ) {
                current_text += "\\";
                c++;
                
            } else if (text[c+1] == '{') { // found the start of an identifier:
                
                if (current_text != ""){
                    tokens.push_back({pp_text_type, current_text, current_line, current_column});
                }
                inside_text = false;
                current_text = "";
                
                current_line = line;
                current_column = column;
                inside_identifier = true;
                c++;
                
            } else if (text[c+1] == '[') { // found the start of a ast_node:
                
                if (current_text != ""){
                    tokens.push_back({pp_text_type, current_text, current_line, current_column});
                }
                inside_text = false;
                current_text = "";
                
                inside_ast_node = true;
                current_line = line;
                current_column = column;
                c++;
                
            } else {
                if (current_text != ""){
                    tokens.push_back({pp_text_type, current_text, current_line, current_column});
                }
                inside_text = false;
                current_text = "";
                
                inside_keyword = true;
                current_line = line;
                current_column = column;
            }
        } else if (text[c] == '\\') {
             if (text[c+1] == '}') { // found the end of an identifer:
                tokens.push_back({pp_identifier_type, current_identifier, current_line, current_column});
                inside_identifier = false;
                current_identifier = "";
                 
                inside_text = true;
                current_line = line;
                current_column = column;
                c++;
                
             } else if (text[c+1] == ']') { // found the end of a ast_node:
                 tokens.push_back({pp_ast_node_type, current_ast_node, current_line, current_column});
                 inside_ast_node = false;
                 current_ast_node = "";
                 
                 inside_text = true;
                 current_line = line;
                 current_column = column;
                 c++;
                 
             }
            
        } else if (inside_keyword && (text[c] == ' ' || text[c] == '\n')) {
            tokens.push_back({pp_keyword_type, current_keyword, current_line, current_column});
            inside_keyword = false;
            current_keyword = "";
            
            inside_text = true;
            current_line = line;
            current_column = column;
            
        } else if (inside_text) {
            current_text += text[c];
            
        } else if (inside_keyword) {
            current_keyword += text[c];
            
        } else if (inside_ast_node) {
            current_ast_node += text[c];
            
        } else if (inside_identifier) {
            current_identifier += text[c];
            
        }
        if (text[c] == '\n') {
            line++;
            column = 0;
        }
    }

    if (current_text != ""){
        tokens.push_back({pp_text_type, current_text, current_line, current_column});
    }
    
    std::cout << "current_identifier: " << current_identifier << "\n";
    std::cout << "current_text: " << current_text << "\n";
    std::cout << "current_ast_node: " << current_ast_node << "\n";
    std::cout << "current_keyword: " << current_keyword << "\n";
    
    print_pp_lex(tokens);
    
    return tokens;
}





// ----------------------------- parser ---------------------------------------







bool pp_parser(const std::vector<struct pp_token> tokens, bool &error) { //todo determin return type of this function.
    
    
    
    
    
    return false;
}











std::string preprocess(std::string text, bool &error) {
    
    text = strip_comments(text, error);    
    auto tokens = pp_lexer(text, error);
    auto ast = pp_parser(tokens, error);
    
    return text;
}
