//
//  error.cpp
//  language
//
//  Created by Daniel Rehman on 1902192.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "error.hpp"

#include "lists.hpp"
#include "color.h"
#include <sstream>
#include <iostream>

#include "analysis_ds.hpp"


#include "llvm/IR/LLVMContext.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/daniels_interpreter/MCJIT.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/GenericValue.h"



// helpers:

static std::string contract_filename(std::string filename) {
    const size_t threshold_length = 30;
    if (filename.size() > threshold_length) {
        std::string shorter(std::find(filename.begin() + (filename.size() - threshold_length), filename.end(), '/'), filename.end());
        shorter.insert(0, "...");
        return shorter;
    } else return filename;
}


static std::string error_heading(const std::string &filename, size_t line, size_t column) {
    std::ostringstream s;
    std::string shorter_filename = contract_filename(filename);
    s << "\n" 
    cBOLD cCYAN << language_name << cRESET 
    cBOLD cGRAY ": " cRESET
    
    cBOLD cMAGENTA << shorter_filename << cRESET 
    cBOLD cWHITE ":" << line << ":" << column << ": " cRESET
    
    cBOLD cBRIGHT_RED "error: " cRESET             
    cBOLD; 
    return s.str();
}

static std::string warning_heading(const std::string &filename, size_t line, size_t column) {
        
    std::ostringstream s;
    std::string shorter_filename = contract_filename(filename);
    s << "\n" 
    cBOLD cCYAN << language_name << cRESET 
    cBOLD cGRAY ": " cRESET
    
    cBOLD cMAGENTA << shorter_filename << cRESET 
    cBOLD cWHITE ":" << line << ":" << column << ": " cRESET
    
    cBOLD cBRIGHT_YELLOW "warning: " cRESET  
    cBOLD; 
    return s.str();
}

static std::string info_heading(const std::string &filename, size_t line, size_t column) {
    
    std::ostringstream s;
    std::string shorter_filename = contract_filename(filename);
    s << "\n" 
    cBOLD cCYAN << language_name << cRESET 
    cBOLD cGRAY ": " cRESET
    
    cBOLD cMAGENTA << shorter_filename << cRESET 
    cBOLD cWHITE ":" << line << ":" << column << ": " cRESET
    
    cBOLD cBRIGHT_BLUE "info: " cRESET   
    cBOLD; 
    return s.str();
}

static std::string note_heading() {
    std::ostringstream s;
    s << "\n" cCYAN << language_name << cRESET cGRAY ": \tnote: " cRESET;
    return s.str();
}

// messagers:

void print_error_message(std::string filename, std::string message, size_t line, size_t column) {
    std::cerr << error_heading(filename, line, column) << message << cRESET << std::endl;
}

void print_warning_message(std::string filename, std::string message, size_t line, size_t column) {
    std::cerr << warning_heading(filename, line, column) << message << std::endl;
}

void print_info_message(std::string filename, std::string message, size_t line, size_t column) {
    std::cerr << info_heading(filename, line, column) << message << std::endl;
}

void print_note(std::string message) {
    std::cerr << note_heading() << message << std::endl;
}



// specialized:

void print_lex_error(std::string filename, std::string state_name, size_t line, size_t column) {
    std::cerr << error_heading(filename, line, column) << "unterminated " << state_name << std::endl;
}

void print_parse_error(std::string filename, size_t line, size_t column, std::string type, std::string found, std::string expected) {
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


void print_llvm_error(const llvm::SMDiagnostic &errors, state &state) { 
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

void print_source_code(std::string text, std::vector<struct token> offending_tokens) {

    syntax_highlight(text);

    auto& t = offending_tokens[0]; //TODO: allow this function to print erros to do with multiple tokens in combintation.
    std::vector<int> offsets = {-2, -1, 0, 1, 2};

    std::string line = "";
    std::istringstream s {text};
    std::vector<std::string> lines = {};
    while (std::getline(s, line)) lines.push_back(line);

    std::cout << "\n";
    for (auto offset : offsets) {
        size_t index = 0;
        if ((int) t.line - 1 + offset >= 0 and (int) t.line - 1 + offset < lines.size()) {
            index = t.line - 1 + offset;
        } else continue;
        
        std::cout << "\t" << cGRAY << t.line + offset << cRESET cGREEN "  │  " cRESET << lines[index] << std::endl;
        
        if (!offset) {
            std::cout << "\t";
            for (int i = 0; i < t.column + 5; i++) std::cout << " ";
            std::cout << cBRIGHT_RED << "^";
            if (t.value.size() > 1) for (int i = 0; i < t.value.size() - 1; i++) std::cout << "~";
            std::cout << cRESET << std::endl;
        }
    }    
    std::cout << std::endl;
}
