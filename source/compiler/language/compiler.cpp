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
#include "codegen.hpp"
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

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>

std::unique_ptr<llvm::Module> frontend(struct file file, llvm::LLVMContext &context) {
    return analyze(correct(parse(file, context), file), context, file);
}

void optimize(llvm::Module* module) {
    // use a pass manager, and string together as many passes as possible.
}


static std::string parse_just_filename(std::string filepath) {
    auto last_dot = filepath.find_last_of(".");
    if (last_dot != std::string::npos) return filepath.substr(0, last_dot);
    else return filepath;
}

std::string generate(llvm::Module* module, const struct file& file) {

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(TargetTriple);
    std::string Error = "";
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    if (!Target) {
        llvm::errs() << Error;
    }

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TheTargetMachine = Target->createTargetMachine(TargetTriple, "generic", "", opt, RM);
    module->setDataLayout(TheTargetMachine->createDataLayout());

    auto object_filemame = file.name + ".o";

    std::error_code error;
    llvm::raw_fd_ostream dest(object_filemame, error, llvm::sys::fs::F_None);
    if (error) {
        llvm::errs() << "Could not open file: " << error.message();
    }

    llvm::legacy::PassManager pass;
    auto filetype = llvm::TargetMachine::CGFT_ObjectFile;
    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, filetype)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        // clean up all other files.
        throw "generate error: cannot emit object file on this target";
    }

    pass.run(*module);
    dest.flush();
    return object_filemame;
}

void link(std::vector<std::string> object_files, const struct arguments& arguments) {

    std::string output_filename = arguments.executable_name;

    std::string link_command = "ld -lc -o " + output_filename + " ";
    for (auto filename : object_files) {
        link_command += filename + " ";
    }

    std::system(link_command.c_str());

    // remove_files(object_files);
}
