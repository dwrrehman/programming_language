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

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/MCJIT.h"

#include "llvm/IR/Verifier.h"
#include "llvm/Linker/Linker.h"

void initialize_llvm() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
}

llvm_module process(const file& file, llvm::LLVMContext& context) {
    return analyze(correct(parse(file), file), file, context);
}

llvm_modules frontend(const arguments& arguments, llvm::LLVMContext& context) {
    llvm_modules modules = {};    
    modules.reserve(arguments.files.size());
    for (auto file : arguments.files) modules.push_back(process(file, context));
    return modules;
}

llvm_module link(llvm_modules&& modules) {
    if (modules.empty()) return {};
    auto result = std::move(modules.back());
    modules.pop_back();
    for (auto& module : modules) if (llvm::Linker::linkModules(*result, std::move(module))) exit(10);
    return result;
}

void set_data_layout(llvm_module& module) {
    auto machine = llvm::EngineBuilder(llvm_module{module.get()}).setEngineKind(llvm::EngineKind::JIT).create();    
    module->setDataLayout(machine->getDataLayout());
}

void interpret(llvm_module& module, const arguments& arguments) {    
    set_data_layout(module); 
    auto jit = llvm::EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::JIT).create();
    jit->finalizeObject();
    exit(jit->runFunctionAsMain(jit->FindFunctionNamed("main"), {arguments.executable_name}, nullptr));
}

void optimize(llvm_module& module) {
    // use a pass manager, and string together as many passes as possible.
}

std::string generate_object_file(llvm_module& module, const arguments& arguments) {
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(TargetTriple);
    std::string Error = "";
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    if (not Target) exit(11);
    
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto CM = llvm::Optional<llvm::CodeModel::Model>();
    auto TheTargetMachine = Target->createTargetMachine(TargetTriple, "generic", "", opt, RM, CM);
    module->setDataLayout(TheTargetMachine->createDataLayout());
    
    auto object_filename = arguments.executable_name + ".o";
    std::error_code error;
    llvm::raw_fd_ostream dest(object_filename, error, llvm::sys::fs::F_None);
    if (error) exit(11);
    
    llvm::legacy::PassManager pass;
    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::TargetMachine::CGFT_ObjectFile)) {
        std::remove(object_filename.c_str());
        exit(11);
    }
    pass.run(*module);
    dest.flush();
    return object_filename;
}

void emit_executable(const std::string& object_file, const arguments& arguments) {
    std::string link_command = "ld -macosx_version_min 10.14 -lSystem -lc -o " + arguments.executable_name + " " + object_file + " ";
    std::system(link_command.c_str());
    if (debug) printf("executable emitted: %s\n", arguments.executable_name.c_str());
    std::remove(object_file.c_str());
}
