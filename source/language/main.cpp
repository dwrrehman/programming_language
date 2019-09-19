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
    initialize_llvm();
    llvm::LLVMContext context;
    auto module = link(frontend(arguments, context));
    if (arguments.use_repl) repl(context);
    if (arguments.use_interpreter) interpret(module, arguments);
    optimize(module);
    emit_executable(generate_object_file(module, arguments), arguments);
}
