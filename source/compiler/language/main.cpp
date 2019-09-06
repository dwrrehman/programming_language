// the main program for "nostril", a n3zqx2l compiler.
#include "arguments.hpp"
#include "compiler.hpp"
#include "interpreter.hpp"
#include "error.hpp"
#include "lists.hpp"

#include "llvm/IR/LLVMContext.h"
#include <cstdlib>

int main(const int argc, const char** argv) {
    auto arguments = get_commandline_arguments(argc, argv);
    if (arguments.error) exit(1);
    if (not arguments.use_repl and arguments.files.empty()) print_error_no_files();
    if (debug) debug_arguments(arguments);
    initialize_llvm();
    llvm::LLVMContext context;
    bool error = false;
    auto modules = frontend(arguments, context, error);
    if (arguments.use_repl) repl(context);
    if (arguments.use_interpreter) exit(interpret(arguments.executable_name, modules));
    optimize(modules);
    auto object_files = generate_object_files(arguments, error, modules);    
    link_and_emit_executable(object_files, arguments);
}
