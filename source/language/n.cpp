// n: a n3zqx2l compiler written in C++.
#include "llvm/AsmParser/Parser.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
static long max_expression_depth = 5;
static bool debug = false;
enum class lex_type {none, string, identifier, llvm_string};
enum class output_type {none, llvm, assembly, object_file, executable};
enum class token_type {null, string, identifier, llvm, operator_};
enum class symbol_type { none, subexpression, string_literal, llvm_literal, identifier };
namespace intrinsic { enum intrinsic_index { typeless, type, infered, llvm, none, application, abstraction, define, evaluate }; }
struct file { const char* name = ""; std::string text = ""; };
struct arguments { std::vector<file> files = {}; enum output_type output = output_type::none; const char* name = ""; };
struct program_data { file file; llvm::Module* module; llvm::IRBuilder<>& builder; };
struct location { long line = 0; long column = 0; };
struct lexing_state { long index = 0; lex_type state = lex_type::none; location at = {}; };
struct token { token_type type = token_type::null; std::string value = ""; location at = {}; };
struct string_literal { token literal = {}; bool error = 0; };
struct llvm_literal { token literal = {}; bool error = 0; };
struct identifier { token name = {}; bool error = 0; };
struct symbol;
struct expression {
    std::vector<symbol> symbols = {};
    long type = 0;
    struct token start = {};
    bool error = false;
};
struct symbol {
    symbol_type type = symbol_type::none;
    expression subexpression = {};
    string_literal string = {};
    llvm_literal llvm = {};
    identifier identifier = {};
    bool error = false;
};
struct resolved_expression {
    long index = 0;
    std::vector<resolved_expression> args = {};
    bool error = false;
};
static inline bool is_identifier(char c) { return isalnum(c) or c == '_'; }
static inline bool is_close_paren(const token& t) { return t.type == token_type::operator_ and t.value == ")"; }
static inline bool is_open_paren(const token& t) { return t.type == token_type::operator_ and t.value == "("; }
static inline bool subexpression(const symbol& s) { return s.type == symbol_type::subexpression; }
static inline bool identifier(const symbol& s) { return s.type == symbol_type::identifier; }
//static inline bool llvm_string(const symbol& s) { return s.type == symbol_type::llvm_literal; }
//static inline bool parameter(const symbol &symbol) { return subexpression(symbol); }
//static inline bool are_equal_identifiers(const symbol &first, const symbol &second) { return identifier(first) and identifier(second) and first.identifier.name.value == second.identifier.name.value; }

static inline arguments get_arguments(const int argc, const char** argv) {
    arguments args = {};
    for (long i = 1; i < argc; i++) {
        const auto word = std::string(argv[i]);
        if (word == "-z") debug = true;
        else if (word == "-u") { printf("usage: n -[zuvrscod/-] <.n/.ll/.o/.s>\n"); exit(0); }
        else if (word == "-v") { printf("n3zqx2l: 0.0.3 \tn: 0.0.3\n"); exit(0); }
        else if (word == "-r" and i + 1 < argc) { args.output = output_type::llvm; args.name = argv[++i]; }
        else if (word == "-s" and i + 1 < argc) { args.output = output_type::assembly; args.name = argv[++i]; }
        else if (word == "-c" and i + 1 < argc) { args.output = output_type::object_file; args.name = argv[++i]; }
        else if (word == "-o" and i + 1 < argc) { args.output = output_type::executable; args.name = argv[++i]; }
        else if (word == "-d" and i + 1 < argc) { auto n = atoi(argv[++i]); max_expression_depth = n ? n : 4; }
        else if (word == "-/") { break; /*the linker argumnets start here.*/ }
        else if (word == "--") { break; /*the interpreter argumnets start here.*/ }
        else if (word[0] == '-') { printf("n: error: bad option: %s\n", argv[i]); exit(1); }
        else {
            std::ifstream stream {argv[i]};
            if (stream.good()) {
                std::string text {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
                stream.close();
                args.files.push_back({argv[i], text});
            } else { printf("n: error: unable to open \"%s\": %s\n", argv[i], strerror(errno)); exit(1); }
        }
    }
    if (args.files.empty()) { printf("n: error: no input files\n"); exit(1); }
    return args;
}

static inline token next(const file& file, lexing_state& lex) {
    token token = {}; auto& at = lex.index; auto& state = lex.state;
    while (lex.index < (long) file.text.size() - 1) {
        const char c = file.text[at], n = file.text[at + 1];
        if (is_identifier(c) and not is_identifier(n) and state == lex_type::none) { token = { token_type::identifier, std::string(1, c), lex.at}; state = lex_type::none; lex.at.column++; at++; return token; }
        else if (c == '\"' and state == lex_type::none) { token = { token_type::string, "", lex.at }; state = lex_type::string; }
        else if (c == '`' and state == lex_type::none) { token = { token_type::llvm, "", lex.at }; state = lex_type::llvm_string; }
        else if (is_identifier(c) and state == lex_type::none) { token = { token_type::identifier, std::string(1, c), lex.at }; state = lex_type::identifier; }
        else if (c == '\\' and state == lex_type::string) {
            if (n == '\\') token.value += "\\";
            else if (n == '\"') token.value += "\"";
            else if (n == 'n') token.value += "\n";
            else if (n == 't') token.value += "\t";
            else { printf("n3zqx2l: error: unknown escape sequence.\n"); exit(1); } ///TODO: make this use the standard error format.
            lex.at.column++; at++;
        } else if ((c == '\"' and state == lex_type::string) or (c == '`' and state == lex_type::llvm_string)) { state = lex_type::none; lex.at.column++; at++; return token; }
        else if (is_identifier(c) and not is_identifier(n) and state == lex_type::identifier) { token.value += c; state = lex_type::none; lex.at.column++; at++; return token; }
        else if (state == lex_type::string or state == lex_type::llvm_string or (is_identifier(c) and state == lex_type::identifier)) token.value += c;
        else if (not is_identifier(c) and not isspace(c) and state == lex_type::none) {
            token = { token_type::operator_, std::string(1, c), lex.at };
            state = lex_type::none; lex.at.column++; at++; return token;
        } if (c == '\n') { lex.at.line++; lex.at.column = 1; } else lex.at.column++; at++;
    }
    if (state == lex_type::string) { printf("n3zqx2l: %s:%ld,%ld: error: unterminated string\n", file.name, lex.at.line, lex.at.column); exit(1); }
    else if (state == lex_type::llvm_string) { printf("n3zqx2l: %s:%ld,%ld: error: unterminated llvm string\n", file.name, lex.at.line, lex.at.column); exit(1); }
    else return { token_type::null, "", lex.at };
}

expression parse(const file& file, lexing_state& state);
static inline symbol parse_symbol(const file& file, lexing_state& state) {
    auto saved = state;
    auto open = next(file, state);
    if (is_open_paren(open)) {
        if (auto e = parse(file, state); not e.error) {
            auto close = next(file, state);
            if (is_close_paren(close)) return {symbol_type::subexpression, e};
            else { printf("n3zqx2l: %s:%ld:%ld: expected \")\"\n", file.name, close.at.line, close.at.column); exit(1); }
        } else state = saved;
    } else state = saved;
    auto t = next(file, state);
    if (t.type == token_type::string) return {symbol_type::string_literal, {}, {t}};
    if (t.type == token_type::llvm) return {symbol_type::llvm_literal, {}, {}, {t}};
    if (t.type == token_type::identifier) return {symbol_type::identifier, {}, {}, {}, {t}};
    else { state = saved; return {symbol_type::none, {}, {}, {}, {}, true}; }
}

expression parse(const file& file, lexing_state& state) {
    std::vector<symbol> symbols = {};
    auto saved = state; auto start = next(file, state); state = saved;
    auto symbol = parse_symbol(file, state);
    while (not symbol.error) {
        symbols.push_back(symbol);
        saved = state;
        symbol = parse_symbol(file, state);
    } state = saved;
    expression result = {symbols};
    result.start = start;
    return result;
}

static inline void set_data_for(std::unique_ptr<llvm::Module>& module) {
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {});
    module->setDataLayout(target_machine->createDataLayout());
}

static inline std::unique_ptr<llvm::Module> generate(expression program, const file& file, llvm::LLVMContext& context) {
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    set_data_for(module);
    llvm::IRBuilder<> builder(context);
    program_data data {file, module.get(), builder};
    std::vector<llvm::ValueSymbolTable> stack {};
    auto main = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(context), {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()}, false), llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main));
    const char* core_name = "/Users/deniylreimn/Documents/projects/n3zqx2l/examples/core.ll";
    struct file core_stdlib = {};
//    open_ll_file(core_name, core_stdlib);
//    parse_ll_file(data, core_stdlib);
//    auto wef = module->getValueSymbolTable();
//    stack.push_back( wef); //NOTE: we will be calling index(wef) when we do a resolve call, only when we NEED to. thqt should be good.
    ///note: insertion doesnt invalidate iterators, so our references to llvm symbol table elements by index is valid, techncally.
    // anwywats. lets do it tomorrow.
//    print_stack(stack);
//    auto resolved = resolve_expression(program, intrinsic::type, main, stack);
    builder.SetInsertPoint(&main->getBasicBlockList().back());
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
    module->print(llvm::outs(), nullptr);
    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) { printf("llvm: %s: error: %s\n", file.name, errors.c_str()); return nullptr; }
//    else if (resolved.error) return nullptr;
    return module;
}

static inline std::vector<std::unique_ptr<llvm::Module>> frontend(const arguments& arguments, llvm::LLVMContext& context) {
    llvm::InitializeAllTargetInfos(); llvm::InitializeAllTargets(); llvm::InitializeAllTargetMCs(); llvm::InitializeAllAsmParsers(); llvm::InitializeAllAsmPrinters();
    std::vector<std::unique_ptr<llvm::Module>> modules = {};
    for (auto file : arguments.files) {
        lexing_state state {0, lex_type::none, {1, 1}};
//        auto saved = state;
//        debug_token_stream(file);
//        print_translation_unit(parse(file, state), file);
//        state = saved;
        modules.push_back(generate(parse(file, state), file, context));
    }
    if (std::find_if(modules.begin(), modules.end(), [](auto& module) { return not module; }) != modules.end()) exit(1);
    return modules;
}

static inline std::unique_ptr<llvm::Module> link(std::vector<std::unique_ptr<llvm::Module>>&& modules) {
    auto result = std::move(modules.back()); modules.pop_back();
    for (auto& module : modules) if (llvm::Linker::linkModules(*result, std::move(module))) exit(1);
    return result;
}

static inline void interpret(std::unique_ptr<llvm::Module> module, const arguments& arguments) {
    auto jit = llvm::EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::JIT).create();
    jit->finalizeObject();
    exit(jit->runFunctionAsMain(jit->FindFunctionNamed("main"), {arguments.name}, nullptr));
}

static inline std::unique_ptr<llvm::Module> optimize(std::unique_ptr<llvm::Module>&& module) { return std::move(module); } ///TODO: unfinished.

static inline void generate_ll_file(std::unique_ptr<llvm::Module> module, const arguments& arguments) {
    std::error_code error;
    llvm::raw_fd_ostream dest(std::string(arguments.name) + ".ll", error, llvm::sys::fs::F_None);
    if (error) exit(1);
    module->print(dest, nullptr);
}

static inline std::string generate_object_file(std::unique_ptr<llvm::Module> module, const arguments& arguments) {
    std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {}); ///TODO: make this not generic!
    auto object_filename = std::string(arguments.name) + ".o";
    std::error_code error;
    llvm::raw_fd_ostream dest(object_filename, error, llvm::sys::fs::F_None);
    if (error) exit(1);
    llvm::legacy::PassManager pass;
    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, llvm::TargetMachine::CGFT_ObjectFile)) { std::remove(object_filename.c_str()); exit(1); }
    pass.run(*module);
    dest.flush();
    return object_filename;
}

static inline void emit_executable(const std::string& object_file, const std::string& exec_name) {    ;
    std::system(std::string("ld -macosx_version_min 10.14 -lSystem -lc -o " + exec_name + " " + object_file).c_str());
    std::remove(object_file.c_str());
}

static inline void output(const arguments& args, std::unique_ptr<llvm::Module>&& module) {
    if (args.output == output_type::none) interpret(std::move(module), args);
    else if (args.output == output_type::llvm) { generate_ll_file(std::move(module), args); }
    else if (args.output == output_type::assembly) { printf("cannot output .s file, unimplemented\n"); /*generate_s_file();*/ }
    else if (args.output == output_type::object_file) generate_object_file(std::move(module), args);
    else if (args.output == output_type::executable) emit_executable(generate_object_file(std::move(module), args), args.name);
}

int main(const int argc, const char** argv) {
    llvm::LLVMContext context;
    const auto args = get_arguments(argc, argv);
    output(args, optimize(link(frontend(args, context))));
}
