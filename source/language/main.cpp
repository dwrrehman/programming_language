#include "arguments.hpp"
#include "compiler.hpp"
#include "interpreter.hpp"

int main(const int argc, const char** argv) {
    auto args = get_commandline_arguments(argc, argv);
    initialize_llvm();
    llvm::LLVMContext context;
    auto module = link(frontend(args, context));
    if (args.use_repl) repl(context);
    if (args.use_interpreter) interpret(module, args);
    optimize(module);
    emit_executable(generate_object_file(module, args), args);
}
