#include "compiler.hpp"
#include "interpreter.hpp"

int main(const int argc, const char** argv) {
    auto args = get_commandline_arguments(argc, argv);
    initialize_llvm();
    llvm::LLVMContext context;
    if (args.use_repl) repl(context);
    auto module = link(frontend(args, context));
    optimize(module);
    if (args.use_interpreter) interpret(module, args);
    else emit_executable(generate_object_file(module, args), args);
}
