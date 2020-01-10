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
#include <iostream>                              // n: a n3zqx2l compiler written in C++.
#include <vector>
enum constants { none, id, op, string, expr,
    action = 'x', exec = 'o', object = 'c', ir = 'i', assembly = 's',
    _name = 3, _declare = 4, _lazy = 6000, _push = 7000, _pop = 8000, _define = 12000, };
struct file { const char* name = ""; std::string text = ""; };
struct arguments { long output = action; const char* name = "a.out"; std::vector<std::string> argv_for_exec = {}; };
struct lexing_state { long index = 0; long state = none; long line = 0; long column = 0; };
struct token { long type = none; std::string value = ""; long line = 0; long column = 0; };
struct expression; struct resolved { long index = 0; std::vector<resolved> args = {}; bool error = false; std::vector<expression> expr = {}; std::string llvm_type = ""; };
struct symbol; struct expression { std::vector<symbol> symbols = {}; resolved type = {}; resolved me = {}; token start = {}; bool error = false; };
struct symbol { long type = none; expression subexpression = {}; token literal = {}; bool error = false; };
struct entry { expression signature = {}; resolved definition = {}; resolved subsitution = {}; };
static inline token next(const file& file, lexing_state& lex) {
    token token = {}; auto& at = lex.index; auto& state = lex.state;
    while (lex.index < (long) file.text.size()) {
        char c = file.text[at], n = at + 1 < (long) file.text.size() ? file.text[at + 1] : 0;
        if (isalnum(c) and not isalnum(n) and state == none) { token = { id, std::string(1, c), lex.line, lex.column}; state = none; lex.column++; at++; return token; }
        else if (c == '\"' and state == none) { token = { string, "", lex.line, lex.column }; state = string; }
        else if (isalnum(c) and state == none) { token = { id, std::string(1, c), lex.line, lex.column }; state = id; }
        else if (c == '\\' and state == string) {
            if (n == '\\') token.value += "\\"; else if (n == '\"') token.value += "\""; else if (n == 'n') token.value += "\n"; else if (n == 't') token.value += "\t";
            else printf("n3zqx2l: %s:%ld:%ld: error: unknown escape sequence '\\%c'\n", file.name, lex.line, lex.column, n); lex.column++; at++;
        } else if ((c == '\"' and state == string)) { state = none; lex.column++; at++; return token; }
        else if (isalnum(c) and not isalnum(n) and state == id) { token.value += c; state = none; lex.column++; at++; return token; }
        else if (state == string or (isalnum(c) and state == id)) token.value += c;
        else if (not isalnum(c) and not isspace(c) and state == none) {
            token = {op, std::string(1, c), lex.line, lex.column};
            state = none; lex.column++; at++; return token;
        } if (c == '\n') { lex.line++; lex.column = 1; } else lex.column++; at++;
    } if (state == string) printf("n3zqx2l: %s:%ld:%ld: error: unterminated string\n", file.name, lex.line, lex.column);
    return {none, "", lex.line, lex.column };
}
static inline expression parse(lexing_state& state, const file& file);
static inline symbol parse_symbol(lexing_state& state, const file& file) {
    auto saved = state; auto open = next(file, state);
    if (open.type == op and open.value == "(") {
        if (auto e = parse(state, file); not e.error) {
            auto close = next(file, state); if (close.type == op and close.value == ")") return {expr, e};
            else { state = saved; printf("n3zqx2l: %s:%ld:%ld: expected \")\"\n", file.name, close.line, close.column); return {expr, e, {}, true}; }
        }
    } state = saved; auto t = next(file, state);
    if (t.type == string) return {string, {}, t};
    if (t.type == id or (t.type == op and t.value != "(" and t.value != ")")) return {id, {}, t};
    else { state = saved;
        //printf("n3zqx2l: %s:%ld:%ld: unexpected \")\"\n", file.name, t.line, t.column);
        return {none, {}, {}, true}; }
}
static inline expression parse(lexing_state& state, const file& file) {
    std::vector<symbol> symbols = {}; auto saved = state; auto start = next(file, state); state = saved;
    auto symbol = parse_symbol(state, file);
    while (not symbol.error) {
        symbols.push_back(symbol); saved = state;
        symbol = parse_symbol(state, file);
    } state = saved; if (symbol.type == expr) return {{}, {}, {}, {}, true};
    expression result = {symbols}; result.start = start; return result;
}
static inline std::string expression_to_string(const expression& given, const std::vector<entry>& entries, long begin = 0, long end = -1, std::vector<resolved> args = {}) {
    std::string result = "("; long i = 0, j = 0;
    for (auto symbol : given.symbols) {
        if (i < begin or (end != -1 and i >= end)) {i++; continue; }
        if (symbol.type == id) result += symbol.literal.value;
        else if (symbol.type == string) result += "\"" + symbol.literal.value + "\"";
        else if (symbol.type == expr and args.empty()) result += "(" + expression_to_string(symbol.subexpression, entries) + ")";
        else if (symbol.type == expr) { result += "(" + expression_to_string(entries[args[j].index].signature, entries, 0, -1, args) + ")"; j++; }
        if (i < (long) given.symbols.size() - 1 and not (i + 1 < begin or (end != -1 and i + 1 >= end))) result += " "; i++;
    } result += ")"; if (given.type.index) result += " " + expression_to_string(entries[given.type.index].signature, entries, 0, -1, given.type.args); return result;
}





static inline void print_resolved_expr(resolved expr, long depth, std::vector<entry> entries, long d = 0);
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
        std::cout << expression_to_string(entry.signature, entries, 0);
            if (entry.subsitution.index) std::cout << " ---> " << std::to_string(entry.subsitution.index);
        if (entry.definition.index) {
            std::cout << " := \n";
            print_resolved_expr(entry.definition, 1, entries);
        }
        std::cout << "\n\n";
//        if (entry.value and show_llvm) {
//            std::cout << "\tLLVM value: ";
//            entry.value->print(llvm::errs());
//        } if (entry.function and show_llvm) {
//            std::cout << "\tLLVM function: ";
//            entry.function->print(llvm::errs());
//        } if (entry.global_variable and show_llvm) {
//            std::cout << "\tLLVM globalvar: ";
//            entry.global_variable->print(llvm::errs());
//        } if (entry.llvm_type and show_llvm) {
//            std::cout << "\tLLVM type (struct): ";
//            entry.llvm_type->print(llvm::errs());
//        }
                
        if (show_llvm) std::cout << "\n\n\n";
        j++;
    } std::cout << "}\n";
}
static inline void print_expression(expression e, int d);
#define prep(x)   for (long i = 0; i < x; i++) std::cout << ".   "
static inline const char* convert_token_type_representation( long type) {
    switch (type) {
        case none: return "{none}";
        case string: return "string";
        case id: return "identifier";
        case op: return "operator";
        case expr: return "subexpr";
            
        default: return "INTERNAL ERROR";
    }
}
static inline void print_symbol(symbol symbol, int d) {
    prep(d); std::cout << "symbol: \n";
    switch (symbol.type) {
        case id:
            prep(d); std::cout << convert_token_type_representation(symbol.literal.type) << ": " << symbol.literal.value << "\n";
            break;
        case string:
            prep(d); std::cout << "string literal: \"" << symbol.literal.value << "\"\n";
            break;
        case expr:
            prep(d); std::cout << "list symbol\n";
            print_expression(symbol.subexpression, d+1);
            break;
        case none:
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
    
    prep(d); std::cout << "type = " << expression.type.index << "\n";
}
static inline void print_resolved_expr(resolved expr, long depth, std::vector<entry> entries, long d) {
    prep(depth); std::cout << d << ": [error = " << std::boolalpha << expr.error << "]\n";
    prep(depth); std::cout << "index = " << expr.index << " :: " << expression_to_string(entries[expr.index].signature, entries, 0);
    std::cout << "\n";
    if (expr.expr.size()) { prep(depth); std::cout << "new signature = \n"; print_expression(expr.expr.front(), depth + 1); std::cout << "\n"; }
    long i = 0;
    for (auto arg : expr.args) {
        prep(depth + 1); std::cout << "argument #" << i++ << ": \n";
        print_resolved_expr(arg, depth + 2, entries, d + 1);
        prep(depth); std::cout << "\n";
    }
    
}








static inline void define(expression& signature, const resolved& type, const resolved& definition, std::vector<entry>& entries, std::vector<std::vector<long>>& stack) {
    stack.back().push_back(signature.me.index = entries.size()); entries.push_back({signature, definition});
    std::stable_sort(stack.back().begin(), stack.back().end(), [&](long a, long b) { return entries[a].signature.symbols.size() > entries[b].signature.symbols.size(); });
}

static inline resolved resolve_at(const expression& given, const resolved& given_type, long& index, long depth, long max_depth, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file);

static inline resolved resolve_expression(const expression& given, const resolved& given_type, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file, long max_depth);

static inline bool equal(resolved a, resolved b, std::vector<entry>& entries) {
    if (entries[a.index].subsitution.index and equal(b, entries[a.index].subsitution, entries)) return true; else if (a.index != b.index or a.args.size() != b.args.size()) return false;
    for (unsigned long i = 0; i < a.args.size(); i++) if (not equal(a.args[i], b.args[i], entries)) return false; return true;
}






static inline bool matches(const expression& given, long signature_index, const expression& signature, const resolved& given_type, std::vector<resolved>& args, long& index, long depth, long max_depth, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file) {
    if (not equal(given_type, signature.type, entries)) return false;
    
    for (auto& symbol : signature.symbols) {
        if (index >= (long) given.symbols.size()) {
            printf("match: returning false, because index >= symbol count...\n"); /////////////3/////////////
            return false;
        }
        if (symbol.type == expr) {
            auto argument = resolve_at(given, symbol.subexpression.type, index, depth + 1, max_depth, entries, stack, file);
            if (argument.error) return false;
            args.push_back({argument});
            entries[symbol.subexpression.me.index].subsitution = argument;
            
        } else if (symbol.type != given.symbols[index].type or
                   symbol.literal.value != given.symbols[index].literal.value) return false;
        
        else index++;
    }
    
    if (index >= (long) given.symbols.size()) {
        printf("match(): we completed the expression, with this matched signature!\n");
//        abort();
    }
    
    
    if (signature_index == _declare) define(args[0].expr.front(), {}, {}, entries, stack);
    
    return true;
}
static expression typify(const expression& given, const resolved& initial_type, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file, long max_depth) {
    if (given.symbols.empty()) return {{}, {}, {}, {}, true};
    
    expression signature = given.symbols.front().subexpression;
    signature.type = initial_type;
    
    for (long i = given.symbols.size(); i-- > 1;)
        signature.type = resolve_expression(given.symbols[i].subexpression, signature.type, entries, stack, file, max_depth);
    
    for (auto& s : signature.symbols)
        if (s.type == expr)
        define(s.subexpression = typify(s.subexpression, {0}, entries, stack, file, max_depth), {}, {}, entries, stack);
    
    return signature;
}

static inline resolved construct_signature(const expression& given, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file, long max_depth) {
    return {3, {}, given.symbols.empty(), {given.symbols.size() and
        given.symbols.front().type == expr
        ? typify(given, {0}, entries, stack, file, max_depth)
        : expression {given.symbols, {1}}}};
}

static inline resolved resolve_at(const expression& given, const resolved& given_type, long& index, long depth, long max_depth, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file) {
    
        
    if (index >= (long) given.symbols.size()) {
        printf("resolve_at(): index was found to be >= symbol count!\n");
        return {0, {}, true};
    }
    
    if (depth > max_depth) {
        printf("resolve_at(): depth > max depth!\n"); //////////////1 2////////////
        return {0, {}, true};
    }
    
    if (given_type.index == _name and given.symbols[index].type == expr)
        return construct_signature(given.symbols[index++].subexpression, entries, stack, file, max_depth);
    
    long saved = index;
    auto saved_stack = stack;
    
    for (auto s : saved_stack.back()) {
        std::vector<resolved> args = {}; index = saved;
        if (matches(given, s, entries[s].signature, given_type, args, index, depth, max_depth, entries, stack, file)) {
            printf("[] "); ///////////////0///////////
            return {s, args};
        }
    }
    
    if (index >= (long) given.symbols.size()) {
        printf("resolve_at(): alternative path for index >= symbol count!\n");
        return {0, {}, true};
    }
    
    if (given.symbols[index].type == expr)
        return resolve_expression(given.symbols[index++].subexpression, given_type, entries, stack, file, max_depth);
    
    
    
    printf("resolve_at(): returning error, because we couldnt do anythign else...\n"); ///////////////////
    return {0, {}, true};
}

static inline resolved resolve_expression(const expression& given, const resolved& given_type, std::vector<entry>& entries, std::vector<std::vector<long>>& stack, const file& file, long max_depth) {
    
    resolved solution {};
    long pointer = 0;
    
//    auto saved_stack = stack; auto saved_entries = entries;
    
//    for (long max_depth = 0; max_depth <= max_expression_depth; max_depth++) {
//        pointer = 0; //entries = saved_entries; stack = saved_stack;
    solution = resolve_at(given, given_type, pointer, 0, max_depth, entries, stack, file);
        
//        if (not solution.error and pointer == (long) given.symbols.size()) break;
//    }
    if (pointer < (long) given.symbols.size()) {
        printf("resolve_expression(): we found the pointer not enough! ERRORING....\n"); ///////////////4///////////
        solution.error = true;
    }
    
    if (solution.error) {
        const auto t = pointer < (long) given.symbols.size() ? given.symbols[pointer].literal : given.start;
        printf("n3zqx2l: %s:%ld:%ld: error: unresolved %s @ %ld : %s ≠ %s\n\n\n",
               file.name, t.line, t.column,
               expression_to_string(given, entries, pointer, pointer + 1).c_str(),
               pointer,
               expression_to_string(given, entries).c_str(),
               expression_to_string(entries[given_type.index].signature, entries, 0, -1, given_type.args).c_str() );
    }

    return solution;
}

static inline resolved resolve(const expression& given, const file& file, long max_depth) {
    
    std::vector<entry> entries {
        {},
        {{{{id,{},{id,"_"}}},{0},{1}}},
        {{{{id,{},{id,"join"}},{expr,{{},{1}}},{expr,{{},{1}}}},{1},{2}}},
        {{{{id,{},{id,"name"}}},{1},{3}}},
        {{{{id,{},{id,"declare"}},{expr,{{},{3}}}},{1},{4}}}
    };
    
    std::vector<std::vector<long>> stack {{2, 4, 1, 3}};
    
    expression pass_through = expression {std::vector<symbol>{
        symbol {expr, {{}, {1}}}
    }, resolved {1}};
        
//    define(pass_through, {}, {}, entries, stack);
    
    resolved given_type = {1};
    auto resolved = resolve_expression(given, given_type, entries, stack, file, max_depth);
    
    /** debug: */
    print_resolved_expr(resolved, 0, entries); printf("\n\n");
    debug(entries, stack, false); printf("\n\n");
    
    if (resolved.error or given.error) exit(1); else return resolved;
}
static inline void set_data_for(std::unique_ptr<llvm::Module>& module) {
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple()); std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {});
    module->setDataLayout(target_machine->createDataLayout());
}
static inline void generate_expression(const resolved& given, llvm::Module* module, llvm::Function* function, llvm::IRBuilder<>& builder) {
    
}
static inline std::unique_ptr<llvm::Module> generate(const resolved& given, const file& file, llvm::LLVMContext& context) {
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    llvm::IRBuilder<> builder(context); set_data_for(module);
    auto main = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(context), {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()}, false), llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main));
    generate_expression(given, module.get(), main, builder);
    builder.SetInsertPoint(&main->getBasicBlockList().back());
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0)); /** debug: */ std::cout << "generating code....:\n"; module->print(llvm::outs(), nullptr);
    std::string errors = ""; if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) { printf("llvm: %s: error: %s\n", file.name, errors.c_str()); exit(1); } else return module;
}
static inline std::unique_ptr<llvm::Module> optimize(std::unique_ptr<llvm::Module>& module) { return std::move(module); } ///TODO: write me.
static inline void interpret(std::unique_ptr<llvm::Module> module, arguments arguments) {
    auto engine = llvm::EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::JIT).create(); engine->finalizeObject();
    exit(engine->runFunctionAsMain(engine->FindFunctionNamed("main"), arguments.argv_for_exec, nullptr));
}
static inline std::string generate_file(std::unique_ptr<llvm::Module> module, const arguments& arguments, llvm::TargetMachine::CodeGenFileType type) {
    std::error_code error; if (type == llvm::TargetMachine::CGFT_Null) { llvm::raw_fd_ostream dest(std::string(arguments.name) + ".ll", error, llvm::sys::fs::F_None); module->print(dest, nullptr); return ""; }
    std::string lookup_error = ""; auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {}); ///TODO: make this not generic!
    auto object_filename = std::string(arguments.name) + (type == llvm::TargetMachine::CGFT_AssemblyFile ? ".s" : ".o");
    llvm::raw_fd_ostream dest(object_filename, error, llvm::sys::fs::F_None); llvm::legacy::PassManager pass;
    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, type)) { std::remove(object_filename.c_str()); exit(1); }
    pass.run(*module); dest.flush();
    return object_filename;
}
static inline void emit_executable(const std::string& object_file, const std::string& exec_name) {
    std::system(std::string("ld -macosx_version_min 10.15 -lSystem -lc -o " + exec_name + " " + object_file).c_str());
    std::remove(object_file.c_str());
}
static inline void output(const arguments& args, std::unique_ptr<llvm::Module>&& module) {
    if (args.output == action) interpret(std::move(module), args);
    else if (args.output == ir) generate_file(std::move(module), args, llvm::TargetMachine::CGFT_Null);
    else if (args.output == object) generate_file(std::move(module), args, llvm::TargetMachine::CGFT_ObjectFile);
    else if (args.output == assembly) generate_file(std::move(module), args, llvm::TargetMachine::CGFT_AssemblyFile);
    else if (args.output == exec) emit_executable(generate_file(std::move(module), args, llvm::TargetMachine::CGFT_ObjectFile), args.name);
}
int main(const int argc, const char** argv) {
    llvm::InitializeAllTargetInfos(); llvm::InitializeAllTargets(); llvm::InitializeAllTargetMCs(); llvm::InitializeAllAsmParsers(); llvm::InitializeAllAsmPrinters();
    llvm::LLVMContext context; auto module = llvm::make_unique<llvm::Module>("_.n", context);
    arguments args = {}; bool use_exec_args = false, no_input = true; long max_depth = 13;
    for (long i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            auto c = argv[i][1];
            if (use_exec_args) args.argv_for_exec.push_back(argv[i]);
            else if (c == '-') use_exec_args = true;
            else if (c == '!') { abort(); /*the linker argumnets start here.*/ }
            else if (c == 'u') { printf("usage: n -[uvxocisd!-] <.n/.ll/.o/.s>\n"); exit(0); }
            else if (c == 'v') { printf("n3zqx2l: 0.0.4 \tn: 0.0.4\n"); exit(0); }
            else if (c == 'd' and i + 1 < argc) { auto n = atol(argv[++i]); if (n) max_depth = n; }
            else if (strchr("xocis", c) and i + 1 < argc) { args.output = c; args.name = argv[++i]; }
            else { printf("n: error: bad option: %s\n", argv[i]); exit(1); }
        } else {
            const char* ext = strrchr(argv[i], '.');
            if (ext && !strcmp(ext, ".n")) {
                std::ifstream stream {argv[i]};
                if (stream.bad()) { printf("n: error: unable to open \"%s\": %s\n", argv[i], strerror(errno)); exit(1); }
                const file file = {argv[i], {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()}};
                lexing_state state {0, none, 1, 1};
                if (llvm::Linker::linkModules(*module, generate(resolve(parse(state, file), file, max_depth), file, context))) exit(1);
            } else if (ext && !strcmp(ext, ".ll")) {
                llvm::SMDiagnostic errors;
                auto m = llvm::parseAssemblyFile(argv[i], errors, context);
                if (not m) { errors.print("llvm", llvm::errs()); exit(1); }
                set_data_for(m);
                if (llvm::Linker::linkModules(*module, std::move(m))) exit(1);
            } else { printf("n: error: cannot process file \"%s\" with extension \"%s\"\n", argv[i], ext); exit(1); }
            no_input = false;
        }
    } if (no_input) { printf("n: error: no input files\n"); exit(1); }
    else output(args, optimize(module));
}
