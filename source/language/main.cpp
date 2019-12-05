#include "compiler.hpp"

int main(const int argc, const char** argv) {
    auto args = get_commandline_arguments(argc, argv);
    initialize_llvm();
    llvm::LLVMContext context;
    auto module = link(frontend(args, context));
    optimize(module);
    if (args.interpret) interpret(module, args);
    else emit_executable(generate_object_file(module, args), args);
}
