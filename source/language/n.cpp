// n: a n3zqx2l compiler written in C++.
#include "llvm/AsmParser/Parser.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"
#include <fstream>
#include <iostream>
#include <vector>
static long max_expression_depth = 10;
enum class output_type {nothing, run, llvm, assembly, object, exec};
enum class type { none, id, string, op, subexpr};
struct file { const char* name = ""; std::string text = ""; };
struct arguments { enum output_type output = output_type::run; const char* name = ""; std::vector<std::string> argv_for_exec = {}; };
struct lexing_state { long index = 0; type state = type::none; long line = 0; long column = 0; };
struct token { type type = type::none; std::string value = ""; long line = 0; long column = 0; };
struct symbol; struct expression { std::vector<symbol> symbols = {}; long type = 0; token start = {}; bool error = false; };
struct symbol { type type = type::none; expression subexpression = {}; token literal = {}; bool error = false; };
struct resolved_expression { long index = 0; std::vector<resolved_expression> args = {}; bool error = false; std::string llvm_type = ""; expression expr = {}; };
struct entry { expression signature = {}; expression definition = {}; llvm::Value* value = nullptr; llvm::Function* function = nullptr; llvm::GlobalVariable* global_variable = nullptr; llvm::Type* llvm_type = nullptr; };
static inline bool is_identifier(char c) { return isalnum(c) or c == '_'; }
static inline bool is_close_paren(const token& t) { return t.type == type::op and t.value == ")"; }
static inline bool is_open_paren(const token& t) { return t.type == type::op and t.value == "("; }
static inline bool equal(const symbol& first, const symbol& second) { return first.type == second.type and first.literal.value == second.literal.value; }
static inline token next(const file& file, lexing_state& lex) {
    token token = {}; auto& at = lex.index; auto& state = lex.state;
    while (lex.index < (long) file.text.size()) {
        char c = file.text[at], n; n = at + 1 < (long) file.text.size() ? file.text[at + 1] : 0;
        if (is_identifier(c) and not is_identifier(n) and state == type::none) { token = { type::id, std::string(1, c), lex.line, lex.column}; state = type::none; lex.column++; at++; return token; }
        else if (c == '\"' and state == type::none) { token = { type::string, "", lex.line, lex.column }; state = type::string; }
        else if (is_identifier(c) and state == type::none) { token = { type::id, std::string(1, c), lex.line, lex.column }; state = type::id; }
        else if (c == '\\' and state == type::string) {
            if (n == '\\') token.value += "\\";
            else if (n == '\"') token.value += "\"";
            else if (n == 'n') token.value += "\n";
            else if (n == 't') token.value += "\t";
            else { printf("n3zqx2l: error(1,1): unknown escape sequence.\n"); } ///TODO: make this use the standard error format, and include linea nd column
            lex.column++; at++;
        } else if ((c == '\"' and state == type::string)) { state = type::none; lex.column++; at++; return token; }
        else if (is_identifier(c) and not is_identifier(n) and state == type::id) { token.value += c; state = type::none; lex.column++; at++; return token; }
        else if (state == type::string or (is_identifier(c) and state == type::id)) token.value += c;
        else if (not is_identifier(c) and not isspace(c) and state == type::none) {
            token = { type::op, std::string(1, c), lex.line, lex.column };
            state = type::none; lex.column++; at++; return token;
        } if (c == '\n') { lex.line++; lex.column = 1; } else lex.column++; at++;
    } if (state == type::string) printf("n3zqx2l: %s:%ld:%ld: error: unterminated string\n", file.name, lex.line, lex.column);
    return { type::none, "", lex.line, lex.column };
}
static inline expression parse(lexing_state& state, const file& file);
static inline symbol parse_symbol(lexing_state& state, const file& file) {
    auto saved = state;
    auto open = next(file, state);
    if (is_open_paren(open)) {
        if (auto e = parse(state, file); not e.error) {
            auto close = next(file, state);
            if (is_close_paren(close)) return {type::subexpr, e};
            else { state = saved; printf("n3zqx2l: %s:%ld:%ld: expected \")\"\n", file.name, close.line, close.column); return {type::subexpr, e, {}, true};}
        } else state = saved;
    } else state = saved;
    auto t = next(file, state);
    if (t.type == type::string) return {type::string, {}, t};
    if (t.type == type::id or (t.type == type::op and not is_open_paren(t) and not is_close_paren(t))) return {type::id, {}, t};
    else { state = saved; return {type::none, {}, {}, true}; }
}
static inline expression parse(lexing_state& state, const file& file) {
    std::vector<symbol> symbols = {};
    auto saved = state; auto start = next(file, state); state = saved;
    auto symbol = parse_symbol(state, file);
    while (not symbol.error ) {
        symbols.push_back(symbol);
        saved = state;
        symbol = parse_symbol(state, file);
    } state = saved;
    if (symbol.type == type::subexpr) return {{}, 0, {}, true};
    expression result = {symbols};
    result.start = start;
    return result;
}
static inline std::string expression_to_string(const expression& given, std::vector<entry> entries, long begin = 0, long end = -1) {
    std::string result = "("; long i = 0;
    for (auto symbol : given.symbols) {
        if (i < begin or (end != -1 and i >= end)) {i++; continue; }
        if (symbol.type == type::id) result += symbol.literal.value;
        else if (symbol.type == type::string) result += "\"" + symbol.literal.value + "\"";
        else if (symbol.type == type::subexpr) result += "(" + expression_to_string(symbol.subexpression, entries) + ")";
        if (i < (long) given.symbols.size() - 1 and not (i + 1 < begin or (end != -1 and i + 1 >= end))) result += " "; i++;
    } result += ")";
    if (given.type) result += " " + expression_to_string(entries[given.type].signature, entries); return result;
}
static inline void define(const entry& e, std::vector<entry>& entries, std::vector<std::vector<long>>& stack) {
    stack.back().push_back(entries.size()); entries.push_back(e);
    std::stable_sort(stack.back().begin(), stack.back().end(), [&](long a, long b) { return entries[a].signature.symbols.size() > entries[b].signature.symbols.size(); });
}
static inline resolved_expression resolve_at(const expression& given, long given_type, long& index, long depth, long max_depth, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file);
static inline resolved_expression resolve_expression(const expression& given, long given_type, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file);
static inline bool matches(const expression& given, long signature_index, const expression& signature, long given_type, std::vector<resolved_expression>& args, long& index, long depth, long max_depth, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file) {
    if (given_type != signature.type) return false;
    for (auto symbol : signature.symbols) {
        if (index >= (long) given.symbols.size()) return false;
        if (symbol.type == type::subexpr) {
            auto argument = resolve_at(given, symbol.subexpression.type, index, depth + 1, max_depth, entries, stack, file);
            if (argument.error) return false;
            args.push_back({argument});
        } else if (not equal(symbol, given.symbols[index])) return false; else index++;
    }
    if (signature_index == 4) define({args[0].expr}, entries, stack);
    if (signature_index == 9991) define({args[0].expr, args[1].expr}, entries, stack);
    else if (signature_index == 9998) stack.push_back(stack.back());
    else if (signature_index == 9999) stack.pop_back();
    return true;
}
static expression typify(expression given, long initial_type, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file) {
    if (given.symbols.empty()) return {{}, 0, {}, true};
    expression signature = given.symbols.front().subexpression;
    given.symbols.erase(given.symbols.begin());
    signature.type = initial_type; for (long i = given.symbols.size(); i--;) signature.type = resolve_expression(given.symbols[i].subexpression, signature.type, entries, stack, file).index;
    stack.push_back(stack.back()); for (auto& s : signature.symbols) if (s.type == type::subexpr) define({s.subexpression = typify(s.subexpression, 0, entries, stack, file)}, entries, stack);
    stack.pop_back(); return signature;
}
static inline resolved_expression construct_signature(expression given, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file) {
    return {3, {}, given.symbols.empty(), "", given.symbols.front().type == type::subexpr ? typify(given, 0, entries, stack, file) : expression {given.symbols, 1}};
}
static inline resolved_expression resolve_at(const expression& given, long given_type, long& index, long depth, long max_depth, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file) {
    if (depth > max_depth or index >= (long) given.symbols.size()) return {0, {}, true};
    if (given_type == 3) {
        if (given.symbols[index].type == type::subexpr) return construct_signature(given.symbols[index++].subexpression, entries, stack, file);
        else return {3, {}, false, "", {{given.symbols[index++]}, 1}};
    } long saved = index;
    auto saved_stack = stack.back();
    for (auto s : saved_stack) {
        std::vector<resolved_expression> args = {}; index = saved;
        if (matches(given, s, entries[s].signature, given_type, args, index, depth, max_depth, entries, stack, file)) return {s, args};
    } if (given.symbols[index].type == type::subexpr) return resolve_expression(given.symbols[index++].subexpression, given_type, entries, stack, file);
    else if (given.symbols[index].type == type::string and given_type == 1) return {0, {}, false, given.symbols[index].literal.value, {} }; else return {0, {}, true};
} /** debug: */ static inline void debug(std::vector<entry> entries, std::vector<std::vector<long>> stack, bool show_llvm);
static inline resolved_expression resolve_expression(const expression& given, long given_type, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file) {
    resolved_expression solution {}; long pointer = 0; auto saved_stack = stack; auto saved_entries = entries;
    for (long max_depth = 0; max_depth <= max_expression_depth; max_depth++) {
        pointer = 0; entries = saved_entries; stack = saved_stack;
        solution = resolve_at(given, given_type, pointer, 0, max_depth, entries, stack, file);
        if (not solution.error and pointer == (long) given.symbols.size()) break;
    } if (pointer < (long) given.symbols.size()) solution.error = true;
    if (solution.error) { debug(entries, stack, false); /// debug
        const auto t = pointer < (long) given.symbols.size() ? given.symbols[pointer].literal : given.start;
        printf("n3zqx2l: %s:%ld:%ld: error: unresolved %s @ %ld : %s\n", file.name, t.line, t.column, expression_to_string(given, entries, pointer, pointer + 1).c_str(), pointer, expression_to_string(given, entries).c_str()); exit(1);
    } return solution;
}

static inline void debug(std::vector<entry> entries, std::vector<std::vector<long>> stack, bool show_llvm) {
    std::cout << "\n\n---- debugging stack: ----\n";
    std::cout << "printing frames: \n";
    for (auto i = 0; i < (long) stack.size(); i++) {
        std::cout << "\t ----- FRAME # "<< i <<"---- \n\t\tidxs: { ";
        for (auto index : stack[i]) {
            std::cout << index << " ";
        } std::cout << "}\n";
    }
    std::cout << "\nmaster: {\n";
    auto j = 0;
    for (auto entry : entries) {
        std::cout << "\t" << std::setw(6) << j << ": ";
        std::cout << expression_to_string(entry.signature, entries, 0) << "\n\n";
        if (entry.value and show_llvm) {
            std::cout << "\tLLVM value: ";
            entry.value->print(llvm::errs());
        } if (entry.function and show_llvm) {
            std::cout << "\tLLVM function: ";
            entry.function->print(llvm::errs());
        } if (entry.global_variable and show_llvm) {
            std::cout << "\tLLVM globalvar: ";
            entry.global_variable->print(llvm::errs());
        } if (entry.llvm_type and show_llvm) {
            std::cout << "\tLLVM type (struct): ";
            entry.llvm_type->print(llvm::errs());
        }
        if (show_llvm) std::cout << "\n\n\n";
        j++;
    } std::cout << "}\n";
}

static inline void print_expression(expression e, int d);
#define prep(x)   for (long i = 0; i < x; i++) std::cout << ".   "
static inline const char* convert_token_type_representation(enum type type) {
    switch (type) {
        case type::none: return "{null}";
        case type::string: return "string";
        case type::id: return "identifier";
        case type::op: return "operator";
        case type::subexpr: return "subexpr";
    }
}

static inline void print_symbol(symbol symbol, int d) {
    prep(d); std::cout << "symbol: \n";
    switch (symbol.type) {
            
        case type::id:
            prep(d); std::cout << convert_token_type_representation(symbol.literal.type) << ": " << symbol.literal.value << "\n";
            break;
            
        case type::string:
            prep(d); std::cout << "string literal: \"" << symbol.literal.value << "\"\n";
            break;
            
        case type::subexpr:
            prep(d); std::cout << "list symbol\n";
            print_expression(symbol.subexpression, d+1);
            break;
            
        case type::none:
            prep(d); std::cout << "{NO SYMBOL TYPE}\n";
            break;
        default: break;
    }
}

static inline void print_expression(expression expression, int d) {
    prep(d); std::cout << "expression: \n";
    prep(d); std::cout << std::boolalpha << "error: " << expression.error << "\n";
    prep(d); std::cout << "symbol count: " << expression.symbols.size() << "\n";
    prep(d); std::cout << "symbols: \n";
    int i = 0;
    for (auto symbol : expression.symbols) {
        prep(d+1); std::cout << i << ": \n";
        print_symbol(symbol, d+1);
        std::cout << "\n";
        i++;
    }
    prep(d); std::cout << "type = " << expression.type << "\n";
}

static inline void print_resolved_expr(resolved_expression expr, long depth, std::vector<entry> entries) {
    prep(depth); std::cout << "[error = " << std::boolalpha << expr.error << "]\n";
    prep(depth); std::cout << "index = " << expr.index << " :: " << expression_to_string(entries[expr.index].signature, entries, 0);
    std::cout << "\n";
    
    if (expr.expr.symbols.size()) { prep(depth); std::cout << "new signature = \n"; print_expression(expr.expr, depth + 1); std::cout << "\n"; }
    
    long i = 0;
    for (auto arg : expr.args) {
        prep(depth + 1); std::cout << "argument #" << i++ << ": \n";
        print_resolved_expr(arg, depth + 2, entries);
        prep(depth); std::cout << "\n";
    }
}

static inline resolved_expression resolve(const expression& given, const file& file) {
    std::vector<entry> entries { /*0:*/{},
        /*1:*/{{{symbol {type::id, {}, {type::id, "_"} } }, 0}}, /*2:*/{{{symbol {type::id, {}, {type::id, "_join"} }, symbol {type::subexpr, {{}, 1}, {}}, symbol {type::subexpr, {{}, 1}, {}}}, 1}}, /*3:*/{{{symbol {type::id, {}, {type::id, "_name"} } }, 1}}, /*4:*/{{{symbol {type::id, {}, {type::id, "_def"} }, symbol {type::subexpr, {{}, 3}, {}}}, 1}},
    }; std::vector<std::vector<long>> stack {{2, 4, 1, 3, 0}};
    auto resolved = resolve_expression(given, 1, entries, stack, file); /** debug: */print_resolved_expr(resolved, 0, entries); printf("\n\n"); debug(entries, stack, false); printf("\n\n");
    if (resolved.error or given.error) exit(1); else return resolved;
}
static inline void set_data_for(std::unique_ptr<llvm::Module>& module) {
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple()); std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {});
    module->setDataLayout(target_machine->createDataLayout());
}
static inline void generate_expression(const resolved_expression& given, llvm::Module* module, llvm::Function* function, llvm::IRBuilder<>& builder) {
    
}
static inline std::unique_ptr<llvm::Module> generate(const resolved_expression& given, const file& file, llvm::LLVMContext& context) {
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    llvm::IRBuilder<> builder(context);
    set_data_for(module);
    auto main = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(context), {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()}, false), llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main));
    generate_expression(given, module.get(), main, builder);
    builder.SetInsertPoint(&main->getBasicBlockList().back());
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0)); /** debug: */ std::cout << "generating code....:\n"; module->print(llvm::outs(), nullptr);
    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) { printf("llvm: %s: error: %s\n", file.name, errors.c_str()); exit(1); }
    else return module;
}
static inline void interpret(std::unique_ptr<llvm::Module> module, arguments arguments) {
    auto engine = llvm::EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::JIT).create(); engine->finalizeObject();
    exit(engine->runFunctionAsMain(engine->FindFunctionNamed("main"), arguments.argv_for_exec, nullptr));
}
static inline std::unique_ptr<llvm::Module> optimize(std::unique_ptr<llvm::Module>& module) { return std::move(module); } ///TODO: write me.
static inline void generate_ll_file(std::unique_ptr<llvm::Module> module, const arguments& arguments) {
    std::error_code error; llvm::raw_fd_ostream dest(std::string(arguments.name) + ".ll", error, llvm::sys::fs::F_None);
    if (error) exit(1); module->print(dest, nullptr);
}
static inline std::string generate_file(std::unique_ptr<llvm::Module> module, const arguments& arguments, llvm::TargetMachine::CodeGenFileType type) {
    std::string lookup_error = ""; auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {}); ///TODO: make this not generic!
    auto object_filename = std::string(arguments.name) + (type == llvm::TargetMachine::CGFT_AssemblyFile ? ".s" : ".o");
    std::error_code error; llvm::raw_fd_ostream dest(object_filename, error, llvm::sys::fs::F_None);
    if (error) exit(1); llvm::legacy::PassManager pass;
    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, type)) { std::remove(object_filename.c_str()); exit(1); }
    pass.run(*module); dest.flush();
    return object_filename;
}
static inline void emit_executable(const std::string& object_file, const std::string& exec_name) {
    std::system(std::string("ld -macosx_version_min 10.14 -lSystem -lc -o " + exec_name + " " + object_file).c_str());
    std::remove(object_file.c_str());
}
static inline void output(const arguments& args, std::unique_ptr<llvm::Module>&& module) {
    if (args.output == output_type::run) interpret(std::move(module), args);
    else if (args.output == output_type::llvm) generate_ll_file(std::move(module), args);
    else if (args.output == output_type::assembly) generate_file(std::move(module), args, llvm::TargetMachine::CGFT_AssemblyFile);
    else if (args.output == output_type::object) generate_file(std::move(module), args, llvm::TargetMachine::CGFT_ObjectFile);
    else if (args.output == output_type::exec) emit_executable(generate_file(std::move(module), args, llvm::TargetMachine::CGFT_ObjectFile), args.name);
}
int main(const int argc, const char** argv) {
    llvm::InitializeAllTargetInfos(); llvm::InitializeAllTargets(); llvm::InitializeAllTargetMCs(); llvm::InitializeAllAsmParsers(); llvm::InitializeAllAsmPrinters();
    llvm::LLVMContext context;
    auto module = llvm::make_unique<llvm::Module>("_.n", context);
    arguments args = {}; bool use_exec_args = false, no_input = true;
    for (long i = 1; i < argc; i++) {
        const auto word = std::string(argv[i]);
        if (use_exec_args) args.argv_for_exec.push_back(word);
        else if (word == "--") use_exec_args = true;
        else if (word == "-u") { printf("usage: n -[uvrscod!-] <.n/.ll/.o/.s>\n"); exit(0); }
        else if (word == "-v") { printf("n3zqx2l: 0.0.3 \tn: 0.0.3\n"); exit(0); }
        else if (word == "-r" and i + 1 < argc) { args.output = output_type::llvm; args.name = argv[++i]; } /// TODO: should this be a "-i" or "-ir", for intermediate code?
        else if (word == "-s" and i + 1 < argc) { args.output = output_type::assembly; args.name = argv[++i]; }
        else if (word == "-c" and i + 1 < argc) { args.output = output_type::object; args.name = argv[++i]; }
        else if (word == "-o" and i + 1 < argc) { args.output = output_type::exec; args.name = argv[++i]; }
        else if (word == "-nothing" and i + 1 < argc) { args.output = output_type::nothing; }
        else if (word == "-d" and i + 1 < argc) { auto n = atoi(argv[++i]); if (n) max_expression_depth = n; }
        else if (word == "-!") { abort(); /*the linker argumnets start here.*/ }
        else if (word[0] == '-') { printf("n: error: bad option: %s\n", argv[i]); exit(1); }
        else {
            auto extension = std::string(strrchr(argv[i], '.'));
            if (extension == ".n") {
                std::ifstream stream {argv[i]};
                if (stream.bad()) { printf("n: error: unable to open \"%s\": %s\n", argv[i], strerror(errno)); exit(1); }
                const file file = {argv[i], {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()}};
                lexing_state state {0, type::none, 1, 1};
                if (llvm::Linker::linkModules(*module, generate(resolve(parse(state, file), file), file, context))) exit(1);
            } else if (extension == ".ll") {
                llvm::SMDiagnostic errors;
                auto m = llvm::parseAssemblyFile(argv[i], errors, context);
                if (not m) { errors.print("llvm", llvm::errs()); exit(1); }
                set_data_for(m);
                if (llvm::Linker::linkModules(*module, std::move(m))) exit(1);
            } else { printf("n: error: cannot process file \"%s\" with extension \"%s\"", argv[i], extension.c_str()); exit(1); }
            no_input = false;
        }
    } if (no_input) { printf("n: error: no input files\n"); exit(1); }
    else if (args.output != output_type::nothing) output(args, optimize(module));
}
