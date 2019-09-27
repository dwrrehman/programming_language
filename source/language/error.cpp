//
//  error.cpp
//  language
//
//  Created by Daniel Rehman on 1902192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "error.hpp"

#include "color.h"
#include "analysis_ds.hpp"

#include "llvm/Support/SourceMgr.h"

#include <sstream>
#include <iostream>
#include <iomanip>

// helpers:

static std::string contract_filename(const std::string& filename) {
    const nat threshold_length = 30;
    if (filename.size() > threshold_length) {
        std::string shorter(std::find(filename.begin() + (filename.size() - threshold_length), filename.end(), '/'), filename.end());
        shorter.insert(0, "...");
        return shorter;
    } else return filename;
}


static std::string error_heading(const std::string& filename, nat line, nat column) {

    std::ostringstream s;
    std::string shorter_filename = contract_filename(filename);
    s << 
    cBOLD cCYAN << language_name << cRESET 
    cBOLD cGRAY ": " cRESET;
    if (filename != "")
        s << cBOLD cMAGENTA << shorter_filename << cGRAY ":" cRESET ;
    
    if (line and column)
        s << cBOLD cWHITE << line << cGRAY ":" cRESET cBOLD cWHITE << column << cGRAY ":" cRESET;
    
    s << cBOLD cBRIGHT_RED " error: " cRESET             
    cBOLD; 
    return s.str();
}

static std::string warning_heading(const std::string& filename, nat line, nat column) {
    std::ostringstream s;
    std::string shorter_filename = contract_filename(filename);
    s << 
    cBOLD cCYAN << language_name << cRESET 
    cBOLD cGRAY ": " cRESET;
    
    if (filename != "")
        s << cBOLD cMAGENTA << shorter_filename << cGRAY ":" cRESET ;
    if (line and column)
        s << cBOLD cWHITE << line << cGRAY ":" cRESET cBOLD cWHITE << column << cGRAY ":" cRESET;
    
    s << cBOLD cBRIGHT_YELLOW " warning: " cRESET  
    cBOLD; 
    return s.str();
}

static std::string info_heading(const std::string& filename, nat line, nat column) {
    
    std::ostringstream s;
    std::string shorter_filename = contract_filename(filename);
    s << 
    cBOLD cCYAN << language_name << cRESET 
    cBOLD cGRAY ": " cRESET;
    
    if (filename != "")
        s << cBOLD cMAGENTA << shorter_filename << cGRAY ":" cRESET ;
    if (line and column)
        s << cBOLD cWHITE << line << cGRAY ":" cRESET cBOLD cWHITE << column << cGRAY ":" cRESET;
    
    s << cBOLD cBRIGHT_BLUE " info: " cRESET   
    cBOLD; 
    return s.str();
}

static std::string note_heading() {
    std::ostringstream s;
    s <<  cCYAN << language_name << cRESET cGRAY ": \tnote: " cRESET;
    return s.str();
}

// messagers:

void print_error_message(const std::string& filename, const std::string& message, nat line, nat column) {
    std::cerr << error_heading(filename, line, column) << message << cRESET << std::endl;
}

void print_warning_message(const std::string& filename, const std::string& message, nat line, nat column) {
    std::cerr << warning_heading(filename, line, column) << message << std::endl;
}

void print_info_message(const std::string& filename, const std::string& message, nat line, nat column) {
    std::cerr << info_heading(filename, line, column) << message << std::endl;
}

void print_note(const std::string& message) {
    std::cerr << note_heading() << message << std::endl;
}



// specialized:

void print_lex_error(const std::string& filename, const std::string& state_name, nat line, nat column) {
    std::cerr << error_heading(filename, line, column) << "unterminated " << state_name << std::endl;
}

void print_parse_error(const std::string& filename, nat line, nat column, 
                       const std::string& type, std::string found,
                       const std::string& expected) {
    if (type == "{null}" or found == "\n" or type == "indent") {

        if (type == "{null}") found = "end of file";
        if (found == "\n") found = "newline";
        if (type == "indent") found = "indent";

        std::cerr << error_heading(filename, line, column) << "unexpected " << found << ", expected " << expected << std::endl;
    } else {
        std::cerr << error_heading(filename, line, column) << "unexpected " << type << ", \"" << found << "\", expected " << expected << std::endl;
    }
}

void print_error_no_files() {
    std::cerr << cWHITE << "nostril" << cRESET cGRAY ": " cRESET cBRIGHT_RED "error" cRESET cGRAY ": " cRESET << "no input files" << std::endl;
    exit(1);
}


void print_llvm_error(const llvm::SMDiagnostic& errors, state &state) { 
    std::cout << 
    cBOLD cGRAY << "llvm: " << cRESET << std::flush;    
    errors.print(state.data.file.name.c_str(), llvm::errs());
    std::cout << std::flush;    
}








void print_latest_analysis_type_error() {
    
}

void set_current_analysis_type_error() {
    
}

void print_undefined_signature_element_error() {
    
}













// source printers:

void syntax_highlight(std::string& text) {
    //TODO: fill in this function.
}

void print_source_code(std::string text, const std::vector<token>& offending_tokens) {

    syntax_highlight(text);

    auto& t = offending_tokens[0]; //TODO: allow this function to print erros to do with multiple tokens in combintation.
    std::vector<int> offsets = {-2, -1, 0, 1, 2};

    std::string line = "";
    std::istringstream s {text};
    std::vector<std::string> lines = {};
    while (std::getline(s, line)) lines.push_back(line);

    std::cout << "\n";
    for (auto offset : offsets) {
        nat index = 0;
        if ((nat) t.line - 1 + offset >= 0 and (nat) t.line - 1 + offset < lines.size()) {
            index = t.line - 1 + offset;
        } else continue;
        
        std::cout << "\t" << cGRAY << std::setw(4) << t.line + offset << cRESET cGREEN " │  " cRESET << lines[index] << std::endl;
        
        if (!offset) {
            std::cout << "\t";
            for (int i = 0; i < t.column + 7; i++) std::cout << " ";
            std::cout << cBRIGHT_RED << "^";
            if (t.value.size() > 1) for (nat i = 0; i < t.value.size() - 1; i++) std::cout << "~";
            std::cout << cRESET << std::endl;
        }
    }    
    std::cout << std::endl;
}
