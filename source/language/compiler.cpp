//
//  compiler.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "compiler.hpp"

#include "lists.hpp"
#include "parser.hpp"
#include "corrector.hpp"
#include "analysis.hpp"

#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/daniels_interpreter/MCJIT.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Linker/Linker.h"

#include <vector>
#include <iostream>


void initialize_llvm() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
}

llvm_module process(file file, llvm::LLVMContext &context) {
    return analyze(correct(parse(file), file), file, context);
}

llvm_modules frontend(arguments arguments, llvm::LLVMContext& context) {
    bool error = false;
    llvm_modules modules = {};    
    modules.reserve(arguments.files.size());
    for (auto file : arguments.files) {
        try {modules.push_back(process(file, context));}
        catch (...) {error = true;}
    }
    if (error) exit(2);
    return modules;
}

llvm_module link(llvm_modules modules) {
    
    if (modules.empty()) return {};
    auto result = std::move(modules.back());
    modules.pop_back();
    
    for (auto& module : modules)         
        if (llvm::Linker::linkModules(*result, std::move(module))) throw "linkModules failure";
    
    return result;
}


void set_data_layout(llvm_module& module) {
    auto machine = llvm::EngineBuilder(llvm_module{module.get()}).setEngineKind(llvm::EngineKind::JIT).create();    
    module->setDataLayout(machine->getDataLayout());
}

void interpret(llvm_module& module, arguments arguments) {    
    set_data_layout(module); 
    auto jit = llvm::EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::JIT).create();
    jit->finalizeObject();
    exit(jit->runFunctionAsMain(jit->FindFunctionNamed("main"), {arguments.executable_name}, nullptr));
}

void optimize(llvm_module& module) {
    // use a pass manager, and string together as many passes as possible.
}

std::string generate_object_file(llvm_module& module, arguments arguments) {
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
    
    auto object_filename = arguments.executable_name + ".o";
    std::error_code error;
    llvm::raw_fd_ostream dest(object_filename, error, llvm::sys::fs::F_None);
    if (error) {throw "generate error: cannot open raw object file";}
    
    llvm::legacy::PassManager pass;
    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::TargetMachine::CGFT_ObjectFile)) {
        std::remove(object_filename.c_str());
        throw "generate error: cannot emit object file on this target";
    }
    pass.run(*module);
    dest.flush();
    return object_filename;
}

void emit_executable(std::string object_file, arguments arguments) {
    std::string link_command = "ld -macosx_version_min 10.14 -lSystem -lc -o " + arguments.executable_name + " " + object_file + " ";
    std::system(link_command.c_str());
    if (debug) std::cout << "executable emitted: " << arguments.executable_name << "\n";
    std::remove(object_file.c_str());
}
