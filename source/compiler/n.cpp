#include "llvm/AsmParser/Parser.h"              // n: a n3zqx2l compiler written in C++.
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"
#include <fstream>
#include <vector>
bool debug = true; enum {action, string, exec = 'o', object = 'c', ir = 'i', assembly = 's', _undefined = 0, _init, _intrinsic_count };
struct file { const char* name = 0; char* text = 0; size_t length = 0; };
struct lexstate { size_t at = 0; size_t in_string = 0; size_t line = 0; size_t column = 0; };
struct token { size_t value = 0; const char* string = 0; size_t line = 0; size_t column = 0; };
struct expr {token literal = {}; std::vector<expr> sub = {}; size_t error = false; };
struct res { size_t index = 0; std::vector<res> args = {}; token literal = {}; size_t error = false; };
struct entry { res signature = {}; res type = {}; res definition = {}; };
static inline token next(lexstate& l, file& file) {
    token t{}; for (auto& at = l.at; at < file.length; at++) {
        const size_t c = file.text[at]; if (c == '\"' && !l.in_string) t = {l.in_string = string, file.text + at + 1, l.line, l.column};
        else if (!isspace(c) && !l.in_string) { at++; return {c, "", l.line, l.column++}; }
        else if (c == '\"' && l.in_string) { l.in_string = false; l.column++; file.text[at++] = '\0'; return t; }
        if (c == '\n') { l.line++; l.column = 1; } else l.column++;
    } if (l.in_string) printf("n3zqx2l: %s:%ld:%ld: error: expected \"\n\n", file.name, l.line, l.column); return t;
} static inline expr parse(lexstate& state, file& file) {
    expr l = {}; auto saved = state; token t = next(state, file); /// if you want to get rid of all @(0,0)'s, then use this code instead:          token t = l.literal = next(state, file); l.literal.value = 0;
    while (t.value && t.value != ')') {
        if (t.value == string || !strchr("()", t.value)) l.sub.push_back({t});
        else if (t.value == '(') {
            auto p = parse(state, file); if (next(state, file).value != ')') printf("n3zqx2l: %s:%ld:%ld: error: expected )\n\n", file.name, t.line, t.column);
            l.sub.push_back(p); t.value = 0; l.sub.back().literal = t;
        } saved = state; t = next(state, file);
    } state = saved; return l;
} static inline void print_expression(const expr& given) {
    if (given.literal.value == string) printf("\"%s\"", given.literal.string);
    else if (given.literal.value) putchar(given.literal.value);
    else { putchar('('); for (const auto& symbol : given.sub) print_expression(symbol); putchar(')'); }
} static inline std::string represent(const res& given, const std::vector<entry>& entries) {
    if (not given.index) return "(null)";
    std::string result = "";
    result += std::to_string(given.index);
    result += ":" + std::string(1, given.literal.value) + ":";
    result += "[";
    for (auto a : given.args) result += represent(a, entries);
    result += "]";
    return result;
}
////////////////////////////////// DEBUG CODE ////////////////////////////////////

void prep(size_t d) { for (size_t i = 0; i < d; i++) printf(".   "); }

static inline void debug_expression(expr e, size_t d) {
    if (not debug) return;
    prep(d); printf("error = %lu\n", e.error);
    prep(d); printf("token: %lu : '%c'  @(%lu, %lu)\n", e.literal.value, (char)e.literal.value, e.literal.line, e.literal.column);
    if (e.literal.value == string) { prep(d); printf("    string = %s\n", e.literal.string); }
    prep(d); printf("children: {\n");
    
    for (auto f : e.sub) {
        prep(d+1); printf("child: \n");
        debug_expression(f, d + 1);
    }
    prep(d); printf("}\n");
}

static inline void debug_intrinsics(std::vector<std::vector<size_t>> intrinsics) {
    if (not debug) return;
    printf("\n---- debugging intrinsics: ----\n");
    
    for (size_t i = 0; i < intrinsics.size(); i++) {
        if (intrinsics[i].empty()) continue;
        
        printf("\t ----- INTRINSIC ID # %lu ---- \n\t\tsignatures: { ", i);
        for (auto index : intrinsics[i]) printf("%lu ",index);
        printf("}\n\n");
    }
    printf("\n--------------------------------\n");
}

static inline void debug_resolved(res e, size_t depth, std::vector<entry> entries, size_t d = 0) {
    if (not debug) return;
    prep(depth); printf("%lu: [error = %lu]\n", d, (size_t) e.error);
    prep(depth); printf("index = %lu\n", e.index);
    prep(depth); printf("literal:   %lu : '%c'  @(%lu, %lu)\n", e.literal.value, (char)e.literal.value, e.literal.line, e.literal.column);
    puts("");
    
    size_t i = 0;
    for (auto arg : e.args) {
        prep(depth + 1); printf("argument #%lu: \n", i++);
        
        debug_resolved(arg, depth + 1, entries, d + 1);
        
        prep(depth + 1); puts("");
    }
}

static void debug_lex(const file &file) {
    if (not debug) return;
    
    printf("-------------- lexing --------------\n");
    lexstate temp_state {0, 0, 1, 1};
    
    struct file temp_file = {file.name, (char*) calloc(file.length, sizeof(char)), file.length};
    for (size_t i = 0; i < file.length; i++) temp_file.text[i] = file.text[i];
    
    token t = next(temp_state, temp_file);
    while (t.value) {
        
        printf("token: %lu : '%c'  @(%lu, %lu)", t.value, (char)t.value, t.line, t.column);
        
        if (t.value == string) printf(" : string: %s\n", t.string);
        else printf("\n");
        
        t = next(temp_state, temp_file);
    }
}

static inline void debug_stack(std::vector<entry> entries, std::vector<std::vector<size_t>> stack) {
    if (not debug) return;
    
    printf("\n---- debugging stack: ----\n");
    printf("printing frames: \n");
    
    for (size_t i = 0; i < stack.size(); i++) {
        
        printf("\t ----- FRAME # %lu ---- \n\t\tidxs: { ", i);
        
        for (auto index : stack[i]) printf("%lu ", index);
        
        puts("}");
    }
    printf("\nmaster: {\n");
    
    size_t j = 0;
    for (auto entry : entries) {
        printf("\t%6lu: ", j);
        
        printf("%s", represent(entry.signature, entries).c_str());
        
        printf("      :      %s", represent(entry.type, entries).c_str());
        
        printf("      =      %s", represent(entry.definition, entries).c_str());
        
        puts("\n");
        j++;
    }
    puts("}");
}
////////////////////////////////// DEBUG CODE ////////////////////////////////////

static inline void define(const res& signature, const res& type, const res& definition, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack) {
    stack.back().push_back(entries.size());
    entries.push_back({signature, type, definition});
    
    ///TODO: fix me:
    //std::stable_sort(stack.back().begin(), stack.back().end(), [&](size_t a, size_t b) { return false;// entries[a].signature.sub.size() > entries[b].signature.sub.size();
    //}); std::stable_sort(stack.back().begin(), stack.back().end(), [&](size_t a, size_t b) { return false; // entries[a].signature.sub.size() and not entries[a].signature.sub.front().literal.value;
    //});
}

static inline bool is_intrin(size_t _class, size_t to_test, const std::vector<std::vector<size_t>>& intrinsics) {
    return std::find(intrinsics[_class].begin(), intrinsics[_class].end(), to_test) != intrinsics[_class].end();
}
static inline bool equal(res a, res b, std::vector<entry>& entries) {
    if (a.index != b.index or a.args.size() != b.args.size()) return false;
    for (unsigned long i = 0; i < a.args.size(); i++) if (not equal(a.args[i], b.args[i], entries)) return false;
    return true;
}
static inline res resolve(const expr& given, const res& given_type, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, const size_t max_depth);
static inline res resolve_at(const expr& given, const res& expected, size_t& index, size_t& best, const size_t depth, const size_t max_depth, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file) {
    if (depth > max_depth) return {0, {}, {}, true}; else if (index < given.sub.size() and not given.sub.at(index).literal.value) return resolve(given.sub.at(index++), expected, entries, stack, intrinsics, file, max_depth);
    auto saved = index; auto saved_stack = stack; for (const auto s : saved_stack.back()) {
        best = std::max(index, best); index = saved; stack = saved_stack; std::vector<res> args = {};
        
        for (size_t j = 0; j < 10; j++) { /// entries.at(s).signature.sub.size()
            
            if (not equal(expected, entries.at(s).type, entries)) goto next;
            ///FIXED:   this is the right place for this.
            ///         we make this test after we do each parameter subsitution.
            
//            const auto& symbol = entries.at(s).signature.sub.at(j);
//            if (index >= given.sub.size()) { if (args.size() and j == 1) return args[0]; else goto next; }
//            if (not symbol.literal.value) {
//                const resolved& argument = resolve_at(given, symbol.type, index, best, depth + 1, max_depth, entries, stack, intrinsics, file);
//                if (argument.error) goto next; args.push_back({argument});
                /// record parameter subsituttion here!
//            } else if (symbol.literal.value != given.sub.at(index).literal.value) goto next; else index++;
        }
        return {s, args}; next: continue;
    } return {0, {}, {}, true};
}
static inline res resolve(const expr& given, const res& given_type, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, const size_t max_depth) {
    if (debug) { printf("-------------- parse tree: -------------------\n");
    debug_expression(given, 0);
    printf("heres the compacted form: \n\n\t");
    print_expression(given);
    printf("\n\n"); }
    size_t index = 0, best = 0;
    res solution = resolve_at(given, given_type, index, best, 0, max_depth, entries, stack, intrinsics, file);
    if (index < given.sub.size()) solution.error = true;
    if (solution.error) {
        const auto b = best < given.sub.size() ? given.sub.at(best) : given;
        debug_expression(b, 0);
        printf("n3zqx2l: %s:%ld:%ld: error: unresolved ", file.name, b.literal.line, b.literal.column); print_expression(b); printf("\n");
    } return solution;
}
static inline void set_data_for(std::unique_ptr<llvm::Module>& module) {
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {});
    module->setDataLayout(target_machine->createDataLayout());
}
static inline llvm::Value* generate_expression(const res& given, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, llvm::Module* module, llvm::Function* function, llvm::IRBuilder<>& builder) {
    return nullptr;
}
static inline std::unique_ptr<llvm::Module> generate(const res& given, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, llvm::LLVMContext& context, bool is_main) {
    if (debug){
        printf("\n\n");
        debug_resolved(given, 0, entries);
        printf("\n\n");
        debug_stack(entries, stack);
        printf("\n\n");
        debug_intrinsics(intrinsics);
        printf("\n\n");
    }
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    llvm::IRBuilder<> builder(context);
    set_data_for(module);
    if (given.error) {
        if (debug) printf("error: not generating llvm: resolution error\n");
        return module;
    }
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
    if (debug) {
        printf(" ------------generating code for module ----------:\n\n");
        module->print(llvm::outs(), nullptr);
    }
    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) printf("llvm: %s: error: %s\n", file.name, errors.c_str());
    return module;
}
static inline std::unique_ptr<llvm::Module> optimize(std::unique_ptr<llvm::Module>& module) {
    if (debug) {
        printf("\n\n\n\n-------- printing the final state of the module before output:------ \n\n");
        module->print(llvm::errs(), nullptr);
    }
    std::string verify_errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(verify_errors) << ""))) printf("llvm: %s: error: %s\n", "init.n", verify_errors.c_str());
    return std::move(module);
}
static inline void interpret(std::unique_ptr<llvm::Module> module, const std::vector<std::string> argv) {
    auto engine = llvm::EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::JIT).create();
    engine->finalizeObject();
    if (debug) printf("info: running...\n");
    if (auto main = engine->FindFunctionNamed("main"); main) exit(engine->runFunctionAsMain(main, argv, nullptr));
    else printf("n: error: could not find entry point\n");
}
static inline std::string generate_file(std::unique_ptr<llvm::Module> module, const char* name, llvm::TargetMachine::CodeGenFileType type) {
    std::error_code error; if (type == llvm::TargetMachine::CGFT_Null) {
        llvm::raw_fd_ostream dest(std::string(name) + ".ll", error, llvm::sys::fs::F_None);
        module->print(dest, nullptr); return "";
    } std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {}); ///TODO: make this not generic!
    auto object_filename = std::string(name) + (type == llvm::TargetMachine::CGFT_AssemblyFile ? ".s" : ".o");
    llvm::raw_fd_ostream dest(object_filename, error, llvm::sys::fs::F_None);
    llvm::legacy::PassManager pass;
    target_machine->addPassesToEmitFile(pass, dest, nullptr, type);
    pass.run(*module); dest.flush(); return object_filename;
}
static inline void emit_executable(const std::string& object_file, const std::string& exec_name) {
    std::system(std::string("ld -macosx_version_min 10.15 -lSystem -lc -o " + exec_name + " " + object_file).c_str());
    std::remove(object_file.c_str());
}
static inline void output(const size_t emit, const char* name, const std::vector<std::string>& args, std::unique_ptr<llvm::Module>&& module) {
    if (emit == action) interpret(std::move(module), args);
    else if (emit == ir) generate_file(std::move(module), name, llvm::TargetMachine::CGFT_Null);
    else if (emit == object) generate_file(std::move(module), name, llvm::TargetMachine::CGFT_ObjectFile);
    else if (emit == assembly) generate_file(std::move(module), name, llvm::TargetMachine::CGFT_AssemblyFile);
    else if (emit == exec) emit_executable(generate_file(std::move(module), name, llvm::TargetMachine::CGFT_ObjectFile), name);
}

static void define_intrinsic(std::string expression, std::vector<entry> &entries, const file &file, std::vector<std::vector<size_t> > &intrinsics, size_t max_depth, std::vector<std::vector<size_t> > &stack) {
//    lexing_state s0 {0, none, 1, 1};
//    auto s = construct_signature(parse(s0, {"_intrinsic.n", expression}), entries, stack, intrinsics, file, max_depth, _name);
//    define(s.name[0], {}, entries, stack);
}

int main(const int argc, const char** argv) {
    llvm::InitializeAllTargetInfos(); llvm::InitializeAllTargets(); llvm::InitializeAllTargetMCs(); llvm::InitializeAllAsmParsers(); llvm::InitializeAllAsmPrinters();
    llvm::LLVMContext context; auto module = llvm::make_unique<llvm::Module>("", context);
    size_t emit = 0, max_depth = 100, use_exec_args = false, no_files = true; const char* emitname = ""; std::vector<std::string> args = {};
    for (long i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            const auto c = argv[i][1]; if (use_exec_args) args.push_back(argv[i]); else if (c == '-') use_exec_args = true;
            else if (c == 'u') { puts("usage: n [-u/-v] [-o <exe>/-c <object>/-i <ir>/-s <assembly>] {[-d <depth>] <.n/.ll/.o/.s>}* [-- {<argv>}*]"); exit(0); }
            else if (c == 'v') { puts("n3zqx2l: 0.0.4 \tn: 0.0.4"); exit(0); }
            else if (c == 'd' and i + 1 < argc) max_depth = atol(argv[++i]);
            else if (strchr("ocis", c) and i + 1 < argc) { emit = c; emitname = argv[++i]; }
            else { printf("n: error: bad option: %s\n", argv[i]); continue; }
        } else {
            const char* ext = strrchr(argv[i], '.'); if (ext && !strcmp(ext, ".n")) {
                FILE* f = fopen(argv[i], "r"); if (not f) { printf("n: %s: error: %s\n", argv[i], strerror(errno)); continue; }
                fseek(f, 0, SEEK_END); const size_t length = ftell(f); char* text = (char*) calloc(length + 1, sizeof(char));
                fseek(f, 0, SEEK_SET); fread(text, sizeof(char), length, f); file file = {argv[i], text, length}; lexstate state {0, 0, 1, 1};
                
                std::vector<entry> entries {{}};
                std::vector<std::vector<size_t>> stack {{}},
                intrinsics(_intrinsic_count, std::vector<size_t>{});
                
                res signature = {}, type = {}, definition = {};
                signature.literal = token {'i'};
                signature.index = 1;
                
                define(signature, type, definition, entries, stack);
                debug_stack(entries, stack);
                debug_lex(file);
                
                if (llvm::Linker::linkModules(*module, generate(resolve(parse(state, file), {_undefined}, entries, stack, intrinsics, file, max_depth), entries, stack, intrinsics, file, context, no_files))) continue;
            } else if (ext && !strcmp(ext, ".ll")) {
                llvm::SMDiagnostic errors; std::string verify_errors = "";
                auto m = llvm::parseAssemblyFile(argv[i], errors, context);
                if (not m) { errors.print("llvm", llvm::errs()); continue; } else set_data_for(m);
                if (llvm::verifyModule(*m, &(llvm::raw_string_ostream(verify_errors) << ""))) { printf("llvm: %s: error: %s\n", argv[i], verify_errors.c_str()); continue; }
                if (llvm::Linker::linkModules(*module, std::move(m))) continue;
            } else { printf("n: error: cannot process file \"%s\" with extension \"%s\"\n", argv[i], ext); continue; }
            no_files = false;
        }
    } if (no_files) printf("n: error: no input files\n"); else output(emit, emitname, args, optimize(module));
}

//                std::vector<std::string> defined_intrinsics {
//                    "(i)",
//                    "(name) (i)",
//                    "(nat) (i)",
//                    "(join ((join-first) (i)) ((join-second) (i))) (i)",
//                    "(decl ((decl-name) (name) (i)) ((decl-ii) (nat) (i)) ((decl-extern) (nat) (i))) (i)"
//                };
                            
///         for (size_t i = _undefined; i < _intrinsic_count; i++) intrinsics[i].push_back(i);

//        if (is_intrin(_declare, s, intrinsics) and args[1].number < _intrinsic_count) intrinsics[args[1].number].push_back(args[0].name[0].me.index = define(args[0].name[0], {}, entries, stack));
//        if (is_intrin(_define, s, intrinsics)) args[0].name[0].me.index = define(args[0].name[0], args[2], entries, stack);


//    _name,
//    _number,
//    _30, _31, _join,
//    _32, _33, _34, _declare,
//
//    _type,


//    auto at = entries.at(given.index);
//    auto sig = represent(at.signature, entries);
//    auto tt = represent(at.type, entries);
//    auto c = std::string(1, given.literal.value);
//    return c; // not fully implemented.

///TODO: we need to throw an error when we encounter a unexpected ")". very important, actually.    at any cost, we need to do it.    it is a silent but deadly nonerror right now.   because it can lead to code not beinginterretered at all.
