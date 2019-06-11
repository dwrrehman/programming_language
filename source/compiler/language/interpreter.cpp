//
//  interpreter.cpp
//  language
//
//  Created by Daniel Rehman on 1902262.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "interpreter.hpp"
#include "compiler.hpp"
#include "parser.hpp"
#include "color.h"
#include "lists.hpp"
#include "arguments.hpp"
#include "error.hpp"
#include "lists.hpp"
#include "lexer.hpp"
#include "nodes.hpp"
#include "analysis.hpp"
#include "corrector.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Linker/Linker.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ADT/Optional.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/AggressiveInstCombine/AggressiveInstCombine.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/daniels_interpreter/Interpreter.h"
#include "llvm/ExecutionEngine/daniels_interpreter/MCJIT.h"

#include <vector>
#include <iostream>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>

static size_t output_line_number = 0;

void print_welcome_message() {
    std::cout << "a " << BRIGHT_CYAN << language_name << RESET << " REPL interpreter " GRAY "(version " << language_version << ").\n" RESET;
    std::cout << "type \":help\" for more information.\n\n";
}

std::string interpreter_prompt(size_t line) {
    return "   " BRIGHT_CYAN "(" RESET WHITE + std::to_string(line) + RESET BRIGHT_CYAN ")" RESET GRAY ": " RESET;
}

std::string output() {
    return "   " BRIGHT_RED "{" RESET GRAY + std::to_string(output_line_number++) + RESET BRIGHT_RED "}" RESET GRAY ": " RESET;
}

static bool is_quit_command(const std::string &line) {
    return line == ":quit" or line == "quit" or line == ":exit" or line == "exit" or line == ":q";
}

void process_repl_command(std::string line) {
    if (line == "clear") {
        std::cout << "\e[1;1H\e[2J";
    } else if (line == "hello") {
        std::cout << output() << "Hello, world!\n";
    } else if (line == "help") {
        std::cout << output() << "REPL commands: \n\t- clear\n\t- hello\n\t- help\n\t- quit\n\n";
    }
}

void analyze(translation_unit unit, std::unique_ptr<llvm::Module>& module, struct file file) {
    // code this after we get the regular one finished.
}

int repl_interpret(std::string executable_name, std::vector<std::unique_ptr<llvm::Module> > &modules) {
    auto & main_module = modules.back();        
    auto jit = llvm::EngineBuilder(std::move(main_module)).setEngineKind(llvm::EngineKind::JIT).create();
    jit->finalizeObject();
    auto fn = jit->FindFunctionNamed("main");                
    const int exit_code = jit->runFunctionAsMain(fn, {executable_name}, nullptr);
    return exit_code;
}

void repl(llvm::LLVMContext& context) {
    print_welcome_message();
    
    srand((unsigned)time(nullptr));
    auto module = llvm::make_unique<llvm::Module>("_REPL", context);
    
    std::string line = "";
    size_t line_number = 0;
    do {
        std::cout << interpreter_prompt(line_number);
        std::getline(std::cin, line);
        if (line != "") line_number++;

        if ((line.size() and line[0] == ':') or is_quit_command(line)) {
            if (is_quit_command(line)) break;
            else {
                line.erase(line.begin(), line.begin() + 1);
                process_repl_command(line);
            }
        } else {                        
            if (line.size() > 10)
                std::cout << output() << "recevied: :::" << line << ":::\n";
        }
    } while (std::cin.good());
    exit(0);
}

