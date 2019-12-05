#include "error.hpp"

#include "color.h"
#include "analysis_ds.hpp"
#include "converters.hpp"

#include "llvm/Support/SourceMgr.h"

#include <sstream>
#include <iostream>
#include <iomanip>

static std::string error_heading(const std::string& filename, nat line, nat column) {

    std::ostringstream s;
    s << 
    cBOLD cCYAN << language_name << cRESET 
    cBOLD cGRAY ": " cRESET;
    if (filename != "")
        s << cBOLD cMAGENTA << filename << cGRAY ":" cRESET ;
    
    if (line and column)
        s << cBOLD cWHITE << line << cGRAY ":" cRESET cBOLD cWHITE << column << cGRAY ":" cRESET;
    
    s << cBOLD cBRIGHT_RED " error: " cRESET             
    cBOLD; 
    return s.str();
}

static std::string warning_heading(const std::string& filename, nat line, nat column) {
    std::ostringstream s;
    s << 
    cBOLD cCYAN << language_name << cRESET 
    cBOLD cGRAY ": " cRESET;
    
    if (filename != "")
        s << cBOLD cMAGENTA << filename << cGRAY ":" cRESET ;
    if (line and column)
        s << cBOLD cWHITE << line << cGRAY ":" cRESET cBOLD cWHITE << column << cGRAY ":" cRESET;
    
    s << cBOLD cBRIGHT_YELLOW " warning: " cRESET  
    cBOLD; 
    return s.str();
}


// messagers:

void print_error_message(const std::string& filename, const std::string& message, nat line, nat column) {
    std::cerr << error_heading(filename, line, column) << message << cRESET << std::endl;
}

void print_warning_message(const std::string& filename, const std::string& message, nat line, nat column) {
    std::cerr << warning_heading(filename, line, column) << message << std::endl;
}


// specialized:

void print_lex_error(const std::string& filename, const std::string& state_name, nat line, nat column) {
    std::cerr << error_heading(filename, line, column) << "unterminated " << state_name << std::endl;
}

void print_parse_error(const std::string& filename, nat line, nat column, const std::string& type, std::string found, const std::string& expected) {
    if (type == "{null}" or found == "\n" or type == "indent") {
        if (type == "{null}") found = "end of file";
        if (found == "\n") found = "newline";
        if (type == "indent") found = "indent";
        std::cerr << error_heading(filename, line, column) << "unexpected " << found << ", expected " << expected << std::endl;
    } else std::cerr << error_heading(filename, line, column) << "unexpected " << type << ", \"" << found << "\", expected " << expected << std::endl;
}

void print_llvm_error(const llvm::SMDiagnostic& errors, state &state) { 
    std::cout << cBOLD cGRAY << "llvm: " << cRESET << std::flush;
    errors.print(state.data.file.name.c_str(), llvm::errs());
    std::cout << std::flush;    
}

void print_unresolved_error(const expression& given, state &state) {
    const std::string name = expression_to_string(given, state.stack);
    print_error_message(state.data.file.name, "unresolved expression: " + name, given.starting_token.line, given.starting_token.column);
}
