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
static long max_expression_depth = 10;
enum class lex_type {none, id, string, llvm};
enum class output_type {none, llvm, assembly, object, exec};
enum class token_type {null, id, string, llvm, op};
enum class symbol_type { none, id, string, llvm, subexpr};
namespace intrinsic { enum intrinsic_index { typeless, type, infered, llvm, none, application, abstraction, define, evaluate }; }
struct file { const char* name = ""; std::string text = ""; };
struct arguments { std::vector<file> files = {}; enum output_type output = output_type::none; const char* name = ""; };
struct program_data { file file; llvm::Module* module; llvm::IRBuilder<>& builder; };
struct lexing_state { long index = 0; lex_type state = lex_type::none; long line = 0; long column = 0; };
struct token { token_type type = token_type::null; std::string value = ""; long line = 0; long column = 0; };
struct symbol; struct expression { std::vector<symbol> symbols = {}; long type = 0; token start = {}; bool error = false; };
struct symbol { symbol_type type = symbol_type::none; expression subexpression = {}; token literal = {}; bool error = false; };
struct resolved_expression { long index = 0; std::vector<resolved_expression> args = {}; bool error = false; };
static inline bool is_identifier(char c) { return isalnum(c) or c == '_'; }
static inline bool is_close_paren(const token& t) { return t.type == token_type::op and t.value == ")"; }
static inline bool is_open_paren(const token& t) { return t.type == token_type::op and t.value == "("; }
static inline bool are_equal_identifiers(const symbol& first, const symbol& second) { return first.type == symbol_type::id and second.type == symbol_type::id and first.literal.value == second.literal.value; }
static inline arguments get_arguments(const int argc, const char** argv) {
    arguments args = {};
    for (long i = 1; i < argc; i++) {
        const auto word = std::string(argv[i]);
        if (word == "-u") { printf("usage: n -[uvrscod/-] <.n/.ll/.o/.s>\n"); exit(0); }
        else if (word == "-v") { printf("n3zqx2l: 0.0.3 \tn: 0.0.3\n"); exit(0); }
        else if (word == "-r" and i + 1 < argc) { args.output = output_type::llvm; args.name = argv[++i]; }
        else if (word == "-s" and i + 1 < argc) { args.output = output_type::assembly; args.name = argv[++i]; }
        else if (word == "-c" and i + 1 < argc) { args.output = output_type::object; args.name = argv[++i]; }
        else if (word == "-o" and i + 1 < argc) { args.output = output_type::exec; args.name = argv[++i]; }
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
    } if (args.files.empty()) { printf("n: error: no input files\n"); exit(1); } else return args;
}

static inline token next(const file& file, lexing_state& lex) {
    token token = {}; auto& at = lex.index; auto& state = lex.state;
    while (lex.index < (long) file.text.size() - 1) {
        const char c = file.text[at], n = file.text[at + 1];
        if (is_identifier(c) and not is_identifier(n) and state == lex_type::none) { token = { token_type::id, std::string(1, c), lex.line, lex.column}; state = lex_type::none; lex.column++; at++; return token; }
        else if (c == '\"' and state == lex_type::none) { token = { token_type::string, "", lex.line, lex.column }; state = lex_type::string; }
        else if (c == '`' and state == lex_type::none) { token = { token_type::llvm, "", lex.line, lex.column }; state = lex_type::llvm; }
        else if (is_identifier(c) and state == lex_type::none) { token = { token_type::id, std::string(1, c), lex.line, lex.column }; state = lex_type::id; }
        else if (c == '\\' and state == lex_type::string) {
            if (n == '\\') token.value += "\\";
            else if (n == '\"') token.value += "\"";
            else if (n == 'n') token.value += "\n";
            else if (n == 't') token.value += "\t";
            else { printf("n3zqx2l: error: unknown escape sequence.\n"); exit(1); } ///TODO: make this use the standard error format.
            lex.column++; at++;
        } else if ((c == '\"' and state == lex_type::string) or (c == '`' and state == lex_type::llvm)) { state = lex_type::none; lex.column++; at++; return token; }
        else if (is_identifier(c) and not is_identifier(n) and state == lex_type::id) { token.value += c; state = lex_type::none; lex.column++; at++; return token; }
        else if (state == lex_type::string or state == lex_type::llvm or (is_identifier(c) and state == lex_type::id)) token.value += c;
        else if (not is_identifier(c) and not isspace(c) and state == lex_type::none) {
            token = { token_type::op, std::string(1, c), lex.line, lex.column };
            state = lex_type::none; lex.column++; at++; return token;
        } if (c == '\n') { lex.line++; lex.column = 1; } else lex.column++; at++;
    } if (state == lex_type::string) { printf("n3zqx2l: %s:%ld,%ld: error: unterminated string\n", file.name, lex.line, lex.column); exit(1); }
    else if (state == lex_type::llvm) { printf("n3zqx2l: %s:%ld,%ld: error: unterminated llvm string\n", file.name, lex.line, lex.column); exit(1); }
    else return { token_type::null, "", lex.line, lex.column };
}

expression parse(const file& file, lexing_state& state);
static inline symbol parse_symbol(const file& file, lexing_state& state) {
    auto saved = state;
    auto open = next(file, state);
    if (is_open_paren(open)) {
        if (auto e = parse(file, state); not e.error) {
            auto close = next(file, state);
            if (is_close_paren(close)) return {symbol_type::subexpr, e};
            else { printf("n3zqx2l: %s:%ld:%ld: expected \")\"\n", file.name, close.line, close.column); exit(1); }
        } else state = saved;
    } else state = saved;
    auto t = next(file, state);
    if (t.type == token_type::string) return {symbol_type::string, {}, t};
    if (t.type == token_type::llvm) return {symbol_type::llvm, {}, t};
    if (t.type == token_type::id) return {symbol_type::id, {}, t};
    else { state = saved; return {symbol_type::none, {}, {}, true}; }
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

static void parse_ll_file(const program_data &data, const file &file) {
    llvm::SMDiagnostic function_errors; llvm::ModuleSummaryIndex my_index(true);
    llvm::MemoryBufferRef reference(file.text, file.name);
    
    if (not llvm::parseAssemblyInto(reference, data.module, &my_index, function_errors)) {
        printf("llvm parse assembly into:  success!\n");
    } else {
        function_errors.print("llvm: ", llvm::errs());
    }
}

static void open_ll_file(const char *core_name, struct file &core_stdlib) {
    std::ifstream stream {core_name};
    if (stream.good()) {
        std::string text {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        stream.close();
        core_stdlib = {core_name, text};
    } else { printf("n: error: unable to open \"%s\": %s\n", core_name, strerror(errno)); exit(1); }
}

static void print_stack(const std::vector<llvm::ValueSymbolTable>& stack) {
    std::cout << "-----------------------printing stack....--------------------------------\n";
    for (auto frame : stack) {
        std::cout << "-------------- printing new frame: ------------\n";
        for (auto& entry : frame) {
            std::string key = entry.getKey();
            llvm::Value* value = entry.getValue();
            std::cout << "key: \"" << key << "\" == value : \n";
            value->print(llvm::outs());
            std::cout << "\n\n";
        }
    }
}

static inline std::unique_ptr<llvm::Module> generate(expression program, const file& file, llvm::LLVMContext& context) {
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple()); std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {});
    module->setDataLayout(target_machine->createDataLayout());
    llvm::IRBuilder<> builder(context); program_data data {file, module.get(), builder};
    std::vector<llvm::ValueSymbolTable> stack {};
    auto main = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(context), {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()}, false), llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main));
    
    const char* core_name = "/Users/deniylreimn/Documents/projects/n3zqx2l/examples/core.ll";
    struct file core_stdlib = {};
    open_ll_file(core_name, core_stdlib);
    parse_ll_file(data, core_stdlib);
    
    stack.push_back(module->getValueSymbolTable());
    print_stack(stack);
    
    for (auto& rgr : stack.back()) {
        auto v = rgr.getValue();
        auto f = v->getValueID();
        std::string b = rgr.getKey();
        std::cout << std::boolalpha;
        std::cout << "function: " << b << " :: " << (f == llvm::Value::FunctionVal) << "\n";
        std::cout << "global: " << b << " :: " << (f == llvm::Value::GlobalVariableVal) << "\n";
        std::cout << "other: " << b << " :: " << (f) << "\n";
    }
    
    
    
    
    
    std::cout << "\n printing types:: \n";
    auto wef = module->getIdentifiedStructTypes();
    for (auto effe : wef) {
        effe->print(llvm::outs());
        std::cout << "\n";
    }
    
    
    auto fwef  = module->getTypeByName("(_)");
    fwef->print(llvm::outs());
    
    
//    auto error = resolve_expression(program, intrinsic::type, main, stack);
    builder.SetInsertPoint(&main->getBasicBlockList().back());
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
    
    std::cout << "\n\n\n\ngenerating code....:\n";
    module->print(llvm::outs(), nullptr); // debug.
    
    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) { printf("llvm: %s: error: %s\n", file.name, errors.c_str()); return nullptr; }
//    else if (error) return nullptr;
    return module;
}

static inline std::vector<std::unique_ptr<llvm::Module>> frontend(const arguments& arguments, llvm::LLVMContext& context) {
    llvm::InitializeAllTargetInfos(); llvm::InitializeAllTargets(); llvm::InitializeAllTargetMCs(); llvm::InitializeAllAsmParsers(); llvm::InitializeAllAsmPrinters();
    std::vector<std::unique_ptr<llvm::Module>> modules = {};
    for (auto file : arguments.files) {
        lexing_state state {0, lex_type::none, 1, 1};
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
    auto jit = llvm::EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::JIT).create(); jit->finalizeObject();
    exit(jit->runFunctionAsMain(jit->FindFunctionNamed("main"), {arguments.name}, nullptr));
}

static inline std::unique_ptr<llvm::Module> optimize(std::unique_ptr<llvm::Module>&& module) { return std::move(module); } ///TODO: write me.

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
    pass.run(*module); dest.flush();
    return object_filename;
}

static inline void emit_executable(const std::string& object_file, const std::string& exec_name) {
    std::system(std::string("ld -macosx_version_min 10.14 -lSystem -lc -o " + exec_name + " " + object_file).c_str());
    std::remove(object_file.c_str());
}

static inline void output(const arguments& args, std::unique_ptr<llvm::Module>&& module) {
    if (args.output == output_type::none) interpret(std::move(module), args);
    else if (args.output == output_type::llvm) { generate_ll_file(std::move(module), args); }
    else if (args.output == output_type::assembly) { printf("cannot output .s file, unimplemented\n"); /*generate_s_file();*/ }
    else if (args.output == output_type::object) generate_object_file(std::move(module), args);
    else if (args.output == output_type::exec) emit_executable(generate_object_file(std::move(module), args), args.name);
}

int main(const int argc, const char** argv) {
    llvm::LLVMContext context;
    const auto args = get_arguments(argc, argv);
    output(args, optimize(link(frontend(args, context))));
}
