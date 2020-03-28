#include "llvm/AsmParser/Parser.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"
#include <fstream>
#include <iostream>                              // n: a n3zqx2l compiler written in C++.
#include <vector>
enum {none, id, string, expr, action = 'x', exec = 'o', object = 'c', ir = 'i', assembly = 's', _undefined = 0, _init, _name, _number, _30, _31, _join, _32, _33, _34, _declare, _type, _intrinsic_count };
struct file { const char* name = ""; std::string text = ""; };
struct arguments { size_t output = action; const char* name = ""; std::vector<std::string> argv_for_exec = {}; };
struct token { char value = 0; std::string string = ""; size_t line = 0; size_t column = 0; };
struct lexing_state { size_t at = 0; size_t state = none; size_t line = 0; size_t column = 0; token t = {}; };
struct resolved { size_t index = 0; std::vector<resolved> args = {}; bool error = false; std::vector<struct expression> name = {}; };
struct expression { std::vector<expression> sub = {}; resolved type = {}; token literal = {}; size_t index = 0; bool error = false; };
struct entry { expression signature = {}; resolved definition = {}; };
static inline token next(lexing_state& l, const file& file) {
    for (auto& at = l.at; at < file.text.size(); at++) {
        char c = file.text[at];
        if (c == '\"' and !l.state) l.t = {string, "", l.line, l.column};
        else if (!isspace(c) and !l.state) { at++; return l.t = {c, "", l.line, l.column++}; }
        else if (c == '\\' and l.state == string) {
            const char n = at+1 < file.text.size() ? file.text[++at] : 0, *valid = "\\\"ntr", *subsitute = "\\\"\n\t\r";
            if (auto i = strchr(valid, n)) l.t.string += subsitute[i - valid];
            else printf("n3zqx2l: %s:%ld:%ld: error: unknown escape sequence '\\%c'\n\n", file.name, l.line, l.column, n); l.column++;
        } else if (c == '\"' and l.state == string) { l.state = none; l.column++; at++; return l.t; }
        else if (l.state == string) l.t.string += c;
        if (c == '\n') { l.line++; l.column = 1; } else l.column++;
    } if (l.state == string) printf("n3zqx2l: %s:%ld:%ld: error: unterminated string\n\n", file.name, l.line, l.column); return l.t = {none, "", l.line, l.column};
} static inline expression parse(lexing_state& state, const file& file) {
    expression l = {}; auto saved = state; token t = next(state, file);
    while (t.value and t.value != ')') {
        if (t.value == string or !strchr("()", t.value)) l.sub.push_back({{}, {}, t});
        else if (t.value == '(') {
            auto p = parse(state, file);
            if (next(state, file).value != ')') printf("n3zqx2l: %s:%ld:%ld: error: expected )\n\n", file.name, t.line, t.column);
            l.sub.push_back(p);
        } saved = state; t = next(state, file);
    } state = saved; return l;
} static inline std::string expression_to_string(const expression& given, const std::vector<entry>& entries, long begin = 0, long end = -1, std::vector<resolved> args = {}) {
    std::string result = "(";
    long i = 0, j = 0;
    for (auto symbol : given.sub) {
        if (i < begin or (end != -1 and i >= end)) { i++; continue; }
        else if (symbol.literal.value == string) result += "\"" + std::string(1, symbol.literal.value) + "\"";
        else if (symbol.literal.value) result += symbol.literal.value;
        else if (not symbol.literal.value and args.empty()) result += "(" + expression_to_string(symbol, entries) + ")";
        else if (not symbol.literal.value) { result += "(" + expression_to_string(entries[args[j].index].signature, entries, 0, -1, args) + ")"; j++; }
    } result += ")";
    if (given.type.index) result += expression_to_string(entries[given.type.index].signature, entries, 0, -1, given.type.args);
    return result;
}

//    resolved subsitution = {}; // very questionable.
// size_t number = 0; ///TODO: subsume me into name, because all intrinsic-nats are actually signatures, technically...?

void prep(size_t d) { for (size_t i = 0; i < d; i++) printf(".   "); }

static inline void print_expression(expression e, size_t d) {
    prep(d); printf("error = %d\n", e.error);
    prep(d); printf("token = %c\n", e.literal.value >= 32 ? e.literal.value: e.literal.value + '0');
    prep(d); printf("token as string = %s\n", e.literal.string.c_str());
    prep(d); printf("children: {\n");
    for (auto f : e.sub) { prep(d+1); printf("child: \n"); print_expression(f, d + 1); }
    prep(d); printf("}\n");
}

static inline void print_intrinsics(std::vector<std::vector<size_t>> intrinsics) {
    std::cout << "\n\n---- debugging intrinsics: ----\n";
        
    for (size_t i = 0; i < intrinsics.size(); i++) {
        if (intrinsics[i].empty()) continue;
        std::cout << "\t ----- INTRINSIC ID # "<< i <<"---- \n\t\tsignatures: { ";
        for (auto index : intrinsics[i]) {
            std::cout << index << " ";
        } std::cout << "}\n";
    }
    std::cout << "\n--------------------------------\n";
}

static inline void print_resolved_expr(resolved expr, size_t depth, std::vector<entry> entries, size_t d = 0) {
    prep(depth); std::cout << d << ": [error = " << std::boolalpha << expr.error << "]\n";
    prep(depth); std::cout << "index = " << expr.index << " :: " << expression_to_string(entries[expr.index].signature, entries, 0) << "\n";

    if (expr.name.size()) {
        prep(depth); std::cout << "expr = " << expression_to_string(expr.name.front(), entries) << "\n";
    }
//    prep(depth); std::cout << "number = " << expr.number << "\n";

    std::cout << "\n";
    long i = 0;
    for (auto arg : expr.args) {
        prep(depth + 1); std::cout << "argument #" << i++ << ": \n";
        print_resolved_expr(arg, depth + 2, entries, d + 1);
        prep(depth); std::cout << "\n";
    }
}


static inline void debug_stack(std::vector<entry> entries, std::vector<std::vector<size_t>> stack) {
    std::cout << "\n\n---- debugging stack: ----\n";
    std::cout << "printing frames: \n";

    for (size_t i = 0; i < stack.size(); i++) {
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
//        if (entry.subsitution.index) std::cout << " ---> " << std::to_string(entry.subsitution.index);
        if (entry.definition.index) {
            std::cout << " := \n";
            print_resolved_expr(entry.definition, 1, entries);
        }
        std::cout << "\n\n";
        j++;
    } std::cout << "}\n";
}



static inline size_t define(expression signature, const resolved& definition, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack) {
    auto index = entries.size(); stack.back().push_back(index);
    signature.index = index; entries.push_back({signature, definition});
    std::stable_sort(stack.back().begin(), stack.back().end(), [&](size_t a, size_t b) { return entries[a].signature.sub.size() > entries[b].signature.sub.size(); });
    std::stable_sort(stack.back().begin(), stack.back().end(), [&](size_t a, size_t b) { return entries[a].signature.sub.size() and not entries[a].signature.sub.front().literal.value; });
    return index;
}

static inline bool is_intrin(size_t _class, size_t to_test, const std::vector<std::vector<size_t>>& intrinsics) {
    return std::find(intrinsics[_class].begin(), intrinsics[_class].end(), to_test) != intrinsics[_class].end();
}

static inline bool equal(resolved a, resolved b, std::vector<entry>& entries) {
//    if (entries[a.index].subsitution.index and equal(b, entries[a.index].subsitution, entries)) return true;
//    else if (entries[b.index].subsitution.index and equal(a, entries[b.index].subsitution, entries)) return true;
//    else if (entries[b.index].subsitution.index and entries[a.index].subsitution.index and equal(entries[a.index].subsitution, entries[b.index].subsitution, entries)) return true;
//    else
        if (a.index != b.index or a.args.size() != b.args.size()) return false;
    for (unsigned long i = 0; i < a.args.size(); i++) if (not equal(a.args[i], b.args[i], entries)) return false;
    return true;
}

static inline resolved resolve(const expression& given, const resolved& given_type, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, size_t max_depth);

static expression typify(const expression& given, const resolved& initial_type, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, size_t max_depth) {
    if (given.sub.empty()) return {{}, {}, {}, {}, true};
    expression signature = given.sub.front();
    for (auto& s : signature.sub) {
        if (not s.literal.value) {
            s = typify(s, {0}, entries, stack, intrinsics, file, max_depth);
            auto k = define(s, {}, entries, stack);
            s.index = k;
            
            ///TODO: we havent realized the right way to do this yet.
            /// it is much more beautiful, and recursive, and parsimonious.
            ///
            ///TODO: Figure it out.
        }
    }
    signature.type = initial_type;
    for (size_t i = given.sub.size(); i-- > 1;) signature.type = resolve(given.sub[i], signature.type, entries, stack, intrinsics, file, max_depth);
    return signature;
}

static inline resolved construct_signature(const expression& given, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, size_t max_depth, size_t its_index) {
    ///TODO: this does unconditional succeeding. write out the logic, long hand, and make typify possibly return an error. very important.
    
    return {its_index, {}, false, {given.sub.size() and not given.sub.front().literal.value ? typify(given, {0}, entries, stack, intrinsics, file, max_depth) : expression {given.sub, {_undefined}}}}; ///TODO: _undefined needs to actually be "_infered". eventually.
}

//static inline resolved construct_number(const expression& given, const file& file, size_t& i, size_t its_index) {
//    const char* string = given.symbols[i].literal.value.c_str();
//    char* pointer = NULL;
//    unsigned long long n = strtoull(string, &pointer, 10);
//    resolved r {its_index, {}, strlen(string) + string != pointer, {}, n};
//    if (r.error) {
//        printf("n3zqx2l: %s:%ld:%ld: error: expected natural number, received \"%s\"\n\n", file.name, given.symbols[i].literal.line, given.symbols[i].literal.column, string);
//        return r;
//    } else { i++; return r; }
//}
//
//static inline std::string convert_to_filename(const expression& e) {
//    std::string result = "";
//    for (auto s : e.sub) if (s.literal.value) result.push_back(s.literal.value);
//    return result;
//}

//resolved load_file(std::vector<resolved>& args, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, size_t max_depth) {
//    auto filename = convert_to_filename(args[0].name[0]);
//    std::ifstream stream {filename};
//    if (not stream.good()) {
//        printf("n: %s: error: could not open input file: %s\n", filename.c_str(), strerror(errno));
//        return {0, {}, true};
//    }
//    const file file = {filename.c_str(), {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()}};
//    lexing_state state {0, none, 1, 1};
//    return resolve(parse(state, file), args[1], entries, stack, intrinsics, file, max_depth);
//}

bool debug = true;

static inline resolved resolve_at(const expression& given, const resolved& expected, size_t& index, size_t& best, size_t depth, size_t max_depth, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file) {
    if (depth > max_depth) return {0, {}, true};

//    else if (index < given.symbols.size() and given.symbols[index].type == expr and is_intrin(_name, expected.index, intrinsics)) return construct_signature(given.symbols[index++].subexpression, entries, stack, intrinsics, file, max_depth, expected.index);
    else if (index < given.sub.size() and not given.sub[index].literal.value) return resolve(given.sub[index++], expected, entries, stack, intrinsics, file, max_depth);
//    else if (index < given.symbols.size() and given.symbols[index].type == id and is_intrin(_number, expected.index, intrinsics)) return construct_number(given, file, index, expected.index);
//    else if (index < given.symbols.size() and given.symbols[index].type == string) return {_string, {}, false, {{{given.symbols[index++]}}}}; /*TODO: should expect type: "(pointer (i8))"     ie, check given_type.args[]... */
//    else if (is_intrin(_lazy, expected.index, intrinsics)) return resolve_at(given, expected.args[0], index, best, depth, max_depth, entries, stack, intrinsics, file);

    auto saved = index; auto saved_stack = stack;
    for (const auto s : saved_stack.back()) {
        best = std::max(index, best); index = saved; stack = saved_stack; std::vector<resolved> args = {};
        
        for (size_t j = 0; j < entries[s].signature.sub.size(); j++) {
            
            const auto& symbol = entries[s].signature.sub[j];
            if (index >= given.sub.size()) { if (args.size() and j == 1) return args[0]; else goto next; }
            if (not symbol.literal.value) {
                resolved argument = resolve_at(given, symbol.type, index, best, depth + 1, max_depth, entries, stack, intrinsics, file);
                if (argument.error) goto next; args.push_back({argument});
                
                if (not symbol.index) abort(); ///DEBUG: leave until we know this doesnt execute.
                
//                entries[symbol.index].subsitution = argument;
            } else if (symbol.literal.value != given.sub[index].literal.value) goto next; else index++;
        }

        if (not equal(expected, entries[s].signature.type, entries)) continue;

//        if (s == _push) stack.push_back(stack.back());
//        if (s == _pop) stack.pop_back();
//        if (is_intrin(_declare, s, intrinsics) and args[1].number < _intrinsic_count) intrinsics[args[1].number].push_back(args[0].name[0].me.index = define(args[0].name[0], {}, entries, stack));
//        if (is_intrin(_define, s, intrinsics)) args[0].name[0].me.index = define(args[0].name[0],  args[2], entries, stack);
//        if (is_intrin(_load, s, intrinsics)) return load_file(args, entries, stack, intrinsics, max_depth);

        return {s, args}; next: continue;
    } return {0, {}, true};
}


static inline resolved resolve(const expression& given, const resolved& given_type, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, size_t max_depth) {

    printf("-------------- parse tree: -------------------\n");
//    print_expression(given, 0);
    printf("heres the compacted form: \n\n\t%s\n\n", expression_to_string(given, entries).c_str());
    
    exit(1);

    size_t index = 0, best = 0;
    resolved solution = resolve_at(given, given_type, index, best, 0, max_depth, entries, stack, intrinsics, file);
    if (index < given.sub.size()) solution.error = true;
    if (solution.error) {
        const auto b = best < given.sub.size() ? given.sub[best].literal : given.literal;
        printf("n3zqx2l: %s:%ld:%ld: error: unresolved %s, expected type %s\n\n", file.name, b.line, b.column, expression_to_string(given, entries, best, best + 1).c_str(), expression_to_string(entries[given_type.index].signature, entries, 0, -1, given_type.args).c_str());
    } return solution;
}

static inline void set_data_for(std::unique_ptr<llvm::Module>& module) {
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {});
    module->setDataLayout(target_machine->createDataLayout());
}

static inline llvm::Value* generate_expression(const resolved& given, std::vector<entry>& entries,
                                               std::vector<std::vector<size_t>>& stack, llvm::Module* module,
                                               llvm::Function* function, llvm::IRBuilder<>& builder) {
    
    ///TODO: code me
    
    return nullptr;
}


static inline std::unique_ptr<llvm::Module> generate(const resolved& given, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, llvm::LLVMContext& context, bool is_main) {
    
    if (debug){
        printf("\n\n");
        print_resolved_expr(given, 0, entries);
        printf("\n\n");
        debug_stack(entries, stack);
          printf("\n\n");
        print_intrinsics(intrinsics);
        printf("\n\n");
    }
          
    if (given.error) exit(1);
    
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    llvm::IRBuilder<> builder(context);
    set_data_for(module);
    
    auto main = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(context), {
        llvm::Type::getInt32Ty(context),
        llvm::Type::getInt8PtrTy(context)->getPointerTo()
    }, false), llvm::Function::ExternalLinkage, "main", module.get());
    
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main));
    
    auto value = generate_expression(given, entries, stack, module.get(), main, builder);
    
    if (debug) {
        printf("the top level value was found to be: \n");
        if (value) value->print(llvm::outs());
        else printf("(null)");
        printf("\n");
    }
        
    builder.SetInsertPoint(&main->getBasicBlockList().back());
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
    
    /** debug: */
    if (debug) {
        std::cout << " ------------generating code for module ----------:\n\n";
        module->print(llvm::outs(), nullptr);
    }
    
    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) { printf("llvm: %s: error: %s\n", file.name, errors.c_str()); exit(1); } else return module;
}

static inline std::unique_ptr<llvm::Module> optimize(std::unique_ptr<llvm::Module>& module) {
    ///TODO: write me.
    
    if (debug) {
        printf("\n\n\n\n-------- printing the final state of the module before output:------ \n\n");
        module->print(llvm::errs(), nullptr);
    }
    
    std::string verify_errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(verify_errors) << ""))) {
        printf("llvm: %s: error: %s\n", "init.n", verify_errors.c_str());
        exit(1);
    }
    
    return std::move(module);
}

static inline void interpret(std::unique_ptr<llvm::Module> module, const arguments& arguments) {
    auto engine = llvm::EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::JIT).create();
    engine->finalizeObject();
    if (auto main = engine->FindFunctionNamed("main"); main) exit(engine->runFunctionAsMain(main, arguments.argv_for_exec, nullptr));
    else { printf("n: error: could not find entry point\n"); exit(1); }
}

static inline std::string generate_file(std::unique_ptr<llvm::Module> module, const arguments& arguments, llvm::TargetMachine::CodeGenFileType type) {
    std::error_code error;
    if (type == llvm::TargetMachine::CGFT_Null) {
        llvm::raw_fd_ostream dest(std::string(arguments.name) + ".ll", error, llvm::sys::fs::F_None);
        module->print(dest, nullptr);
        return "";
    }
    std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {}); ///TODO: make this not generic!
    
    auto object_filename = std::string(arguments.name) + (type == llvm::TargetMachine::CGFT_AssemblyFile ? ".s" : ".o");
    llvm::raw_fd_ostream dest(object_filename, error, llvm::sys::fs::F_None);
    
    llvm::legacy::PassManager pass;
    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, type)) {
        std::remove(object_filename.c_str());
        exit(1);
    }
    pass.run(*module);
    dest.flush();
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

//static void define_intrinsic(std::string expression, std::vector<entry> &entries, const file &file, std::vector<std::vector<size_t> > &intrinsics, size_t max_depth, std::vector<std::vector<size_t> > &stack) {
//    lexing_state s0 {0, none, 1, 1};
//    auto s = construct_signature(parse(s0, {"_intrinsic.n", expression}), entries, stack, intrinsics, file, max_depth, _name);
//    define(s.name[0], {}, entries, stack);
//}

int main(const int argc, const char** argv) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    llvm::LLVMContext context;
    auto module = llvm::make_unique<llvm::Module>("0u6agpc3li0rkpw1a1xs13b.n", context);
    arguments args = {};
    bool use_exec_args = false, no_files = true;
    size_t max_depth = 100;
    for (long i = 1; i < argc; i++) {
        
        if (argv[i][0] == '-') {
            const auto c = argv[i][1];
            
            if (use_exec_args) args.argv_for_exec.push_back(argv[i]);
            
            else if (c == '-') use_exec_args = true;
            
            else if (c == 'u') {
                puts("usage: n [-u/-v] [-o <exe>/-c <object>/-i <ir>/-s <assembly>] [-d <depth>] <.n/.ll/.o/.s> [-- <argv>]");
                exit(0);
            } else if (c == 'v') {
                puts("n3zqx2l: 0.0.4 \tn: 0.0.4");
                exit(0);
                
            } else if (c == 'd' and i + 1 < argc)
                max_depth = atol(argv[++i]);
            
            else if (strchr("ocis", c) and i + 1 < argc) {
                args.output = c;
                args.name = argv[++i];
                
            } else {
                printf("n: error: bad option: %s\n", argv[i]);
                exit(1);
            }
        } else {
            const char* ext = strrchr(argv[i], '.');
            if (ext && !strcmp(ext, ".n")) {
                std::ifstream stream {argv[i]};
                if (not stream.good()) {
                    printf("n: %s: error: could not open input file: %s\n", argv[i], strerror(errno));
                    exit(1);
                }
                const file file = {argv[i], {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()}};
                lexing_state state {0, none, 1, 1};
                
                // 1kqfsnyh5t3hr8viagrr6      2tsrb944gazx3a8cqy2g9     3q1c0pzkzhu2l9t8j6h7a    4lco2hyh80iwtimpq7o58    5we9uq5txfjqjgkeb2chb
                
//                std::vector<std::string> defined_intrinsics { "(i)", "(name) (i)", "(nat) (i)", "(join ((join-first) (i)) ((join-second) (i))) (i)", "(decl ((decl-name) (name) (i)) ((decl-ii) (nat) (i)) ((decl-extern) (nat) (i))) (i)"};
                std::vector<entry> entries {{}};
                std::vector<std::vector<size_t>> stack {{}}, intrinsics(_intrinsic_count, std::vector<size_t>{});
                for (size_t i = _undefined; i < _type; i++) intrinsics[i].push_back(i);
//                for (auto s : defined_intrinsics) define_intrinsic(s, entries, file, intrinsics, max_depth, stack);
                
//                debug_stack(entries, stack);
                
                if (llvm::Linker::linkModules(*module, generate(resolve(parse(state, file), {_init}, entries, stack, intrinsics, file, max_depth), entries, stack, intrinsics, file, context, no_files))) exit(1);
                
            } else if (ext && !strcmp(ext, ".ll")) {
                
                llvm::SMDiagnostic errors;
                std::string verify_errors = "";
                auto m = llvm::parseAssemblyFile(argv[i], errors, context);
                if (not m) {
                    errors.print("llvm", llvm::errs());
                    exit(1);
                } else set_data_for(m);
                
                if (llvm::verifyModule(*m, &(llvm::raw_string_ostream(verify_errors) << ""))) {
                    printf("llvm: %s: error: %s\n", argv[i], verify_errors.c_str());
                    exit(1);
                }
                
                if (llvm::Linker::linkModules(*module, std::move(m)))
                    exit(1);
            } else {
                printf("n: error: cannot process file \"%s\" with extension \"%s\"\n", argv[i], ext);
                exit(1);
            }
            no_files = false;
        }
    }
    if (no_files) printf("n: error: no input files\n");
    else output(args, optimize(module));
}

//        if (t.type) {
//            printf("n3zqx2l: %s:%ld:%ld: error: unexpected \"%s\"\n\n", file.name, t.line, t.column, t.value.c_str());
//        }
