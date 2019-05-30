//
//  compiler.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "compiler.hpp"
#include "arguments.hpp"
#include "lexer.hpp"
#include "parser.hpp"
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

#include "arguments.hpp"
#include "compiler.hpp"
#include "interpreter.hpp"
#include "error.hpp"
#include "lists.hpp"

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

void initialize_llvm() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
}

std::unique_ptr<llvm::Module> proccess(struct file file, llvm::LLVMContext &context) {
    return analyze(correct(parse(file, context), file), context, file);
}

int interpret(std::string executable_name, std::vector<std::unique_ptr<llvm::Module> > &modules) {
    auto & main_module = modules.back();        
    auto jit = llvm::EngineBuilder(std::move(main_module)).setEngineKind(llvm::EngineKind::JIT).create();
    jit->finalizeObject();
    auto fn = jit->FindFunctionNamed("main");                
    const int exit_code = jit->runFunctionAsMain(fn, {executable_name}, nullptr);
    return exit_code;
}

std::vector<std::string> generate_object_files(const struct arguments& arguments, bool error, std::vector<std::unique_ptr<llvm::Module>>& modules) {
    size_t i = 0;
    std::vector<std::string> object_files = {};
    object_files.reserve(modules.size());
    for (auto& module : modules) {        
        try {object_files.push_back(generate(module, arguments.files[i++]));}  
        catch(...) {error = true;}
    }
    if (error) {
        delete_files(object_files); 
        exit(3);        
    } else return object_files;
}

std::vector<std::unique_ptr<llvm::Module>> frontend(const struct arguments &arguments, llvm::LLVMContext& context, bool error) {
    std::vector<std::unique_ptr<llvm::Module>> modules = {};    
    modules.reserve(arguments.files.size());    
    for (auto file : arguments.files) {
        try {modules.push_back(proccess(file, context));}
        catch (...) {error = true;}
    }
    if (error) exit(2);
    return modules;
}

void optimize(std::vector<std::unique_ptr<llvm::Module>>& modules) {
    for (auto& module : modules) {
        // use a pass manager, and string together as many passes as possible.
    }
}

std::string generate(std::unique_ptr<llvm::Module>& module, const struct file& file) {
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(TargetTriple);
    std::string Error = "";
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    if (!Target) {throw "generate error: Target Registry: " + Error;}
    
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto CM = llvm::Optional<llvm::CodeModel::Model>();
    auto TheTargetMachine = Target->createTargetMachine(TargetTriple, "generic", "", opt, RM, CM);
    module->setDataLayout(TheTargetMachine->createDataLayout());
    
    auto object_filemame = file.name + ".o";
    std::error_code error;
    llvm::raw_fd_ostream dest(object_filemame, error, llvm::sys::fs::F_None);
    if (error) {throw "generate error: cannot open raw object file";}

    llvm::legacy::PassManager pass;
    auto filetype = llvm::TargetMachine::CGFT_ObjectFile;
    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, filetype)) {
        delete_files({object_filemame});
        throw "generate error: cannot emit object file on this target";
    }
    pass.run(*module);
    dest.flush();
    return object_filemame;
}

void delete_files(std::vector<std::string> object_filenames) {
    for (auto file : object_filenames) {
        std::remove(file.c_str());
    }
}

void link_and_emit_executable(std::vector<std::string> object_files, const struct arguments& arguments) {
    std::string output_filename = arguments.executable_name;
    std::string link_command = "ld -macosx_version_min 10.14 -lSystem -lc -o " + output_filename + " ";
    for (auto filename : object_files) {
        link_command += filename + " ";
    }
    std::system(link_command.c_str());
    if (debug) std::cout << "executable emitted: " << output_filename << "\n";
    delete_files(object_files);
}
