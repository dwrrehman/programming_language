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
static long max_expression_depth = 5;
enum class output_type {nothing, run, llvm, assembly, object, exec};
enum class type { none, id, string, op, subexpr};
struct file { const char* name = ""; std::string text = ""; };
struct arguments { std::vector<file> files = {}; enum output_type output = output_type::run; const char* name = ""; };
struct program { file file; llvm::Module* module; llvm::IRBuilder<>& builder; };
struct lexing_state { long index = 0; type state = type::none; long line = 0; long column = 0; };
struct token { type type = type::none; std::string value = ""; long line = 0; long column = 0; };
struct symbol; struct expression { std::vector<symbol> symbols = {}; long type = 0; token start = {}; bool error = false; llvm::Type* llvm_type = nullptr; };
struct symbol { type type = type::none; expression subexpression = {}; token literal = {}; bool error = false; };
struct resolved_expression { long index = 0; std::vector<resolved_expression> args = {}; bool error = false; llvm::Type* llvm_type = nullptr; expression expr = {}; };
struct entry { expression signature = {}; llvm::Value* value = nullptr; llvm::Function* function = nullptr; llvm::GlobalVariable* global_variable = nullptr; llvm::Type* llvm_type = nullptr; };
static inline bool is_identifier(char c) { return isalnum(c) or c == '_'; }
static inline bool is_close_paren(const token& t) { return t.type == type::op and t.value == ")"; }
static inline bool is_open_paren(const token& t) { return t.type == type::op and t.value == "("; }
static inline bool are_equal_identifiers(const symbol& first, const symbol& second) { return first.type == type::id and second.type == type::id and first.literal.value == second.literal.value; }
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
static inline expression parse(const file& file, lexing_state& state, long d);
static inline symbol parse_symbol(const file& file, lexing_state& state, long d) {
    auto saved = state;
    auto open = next(file, state);
    if (is_open_paren(open)) {
        if (auto e = parse(file, state, d+2); not e.error) {
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
static inline expression parse(const file& file, lexing_state& state, long d) {
    std::vector<symbol> symbols = {};
    auto saved = state; auto start = next(file, state); state = saved;
    auto symbol = parse_symbol(file, state,d+1);
    while (not symbol.error ) {
        symbols.push_back(symbol);
        saved = state;
        symbol = parse_symbol(file, state, d+2);
    } state = saved;
    if (symbol.type == type::subexpr) return {{}, 0, {}, true};
    expression result = {symbols};
    result.start = start;
    return result;
}

#define prep(x)   for (long i = 0; i < x; i++) std::cout << ".   "

static inline void debug(struct symbol_table stack, bool show_llvm = false);
static inline void print_expression(expression e, int d);

static inline std::string expression_to_string(const expression& given, struct symbol_table& stack, long begin = 0, long end = -1);

struct symbol_table { ///TODO: remove this data structure. we can just pass around the two members together, or put them in the program_data variable.
    std::vector<entry> master = {{}};
    std::vector<std::vector<long>> frames = {{0}};
    void push() { frames.push_back({frames.back()}); }
    void pop() { for (auto i : top()) master.erase(master.begin() + i); frames.pop_back(); }
    std::vector<long>& top() { return frames.back(); }
    expression& get(long index) { return master[index].signature; }
    long lookup(std::string key) { return std::distance(master.begin(), std::find_if(master.begin(), master.end(), [&](const entry& entry) { return key == expression_to_string(entry.signature, *this, 0);})); }
    bool contains(std::string key) { return lookup(key) < (long) master.size(); }
    void define(const entry& e) { top().push_back(master.size()); master.push_back(e); std::stable_sort(top().begin(), top().end(), [&](long a, long b) { return get(a).symbols.size() > get(b).symbols.size(); });}
};

static inline std::string resolved_expression_to_string(const resolved_expression& given, symbol_table& stack) {
    std::string result = "("; long i = 0;
    for (auto symbol : stack.master[given.index].signature.symbols) {
        if (symbol.type == type::id) result += symbol.literal.value;
        else if (symbol.type == type::string) result += "\"" + symbol.literal.value + "\"";
        else if (symbol.type == type::subexpr) result += "(" + expression_to_string(symbol.subexpression, stack) + (i < (long) given.args.size() ? "=" + resolved_expression_to_string(given.args[i], stack) : "") + ")";
        if (i++ < (long) stack.master[given.index].signature.symbols.size() - 1) result += " ";
    } result += ")";
    if (stack.master[given.index].signature.type) result += " " + expression_to_string(stack.master[stack.master[given.index].signature.type].signature, stack);
    return result;
}

static inline std::string expression_to_string(const expression& given, symbol_table& stack, long begin, long end) {    
    std::string result = "("; long i = 0;
    for (auto symbol : given.symbols) {
        if (i < begin or (end != -1 and i >= end)) {i++; continue; }
        if (symbol.type == type::id) result += symbol.literal.value;
        else if (symbol.type == type::string) result += "\"" + symbol.literal.value + "\"";
        else if (symbol.type == type::subexpr) result += "(" + expression_to_string(symbol.subexpression, stack) + ")";
        if (i < (long) given.symbols.size() - 1 and not (i + 1 < begin or (end != -1 and i + 1 >= end))) result += " "; i++;
    } result += ")";
    if (given.llvm_type) {
        std::string type = "";
        given.llvm_type->print(llvm::raw_string_ostream(type) << "", false, true);
        result += " (``" + type + "``) (_)";        
    } else if (given.type) result += " " + expression_to_string(stack.master[given.type].signature, stack); return result;
}

static inline resolved_expression resolve(const expression& given, long given_type, llvm::Function*& function, long& index, long depth, long max_depth, program& data, symbol_table& stack, long gd);

static inline bool matches(const expression& given, const expression& signature, long given_type, std::vector<resolved_expression>& args, llvm::Function*& function, long& index, long depth, long max_depth, program& data, symbol_table stack, long gd) {
    
//    prep(depth + gd); std::cout << "calling matches(" << expression_to_string(given, stack) << "," << expression_to_string(signature, stack) << ") ...\n";
    if (given_type != signature.type and given_type != 3) {
//        prep(depth + gd + 1); std::cout << "   ----> false(0)!\n";
        return false; }
    for (auto symbol : signature.symbols) {
        if (index >= (long) given.symbols.size()) { // this line of code might be why we cant do empty signatures?
//            prep(depth + gd + 1); std::cout << "   ----> false(1)!\n";
            return false;
        }
        if (symbol.type == type::subexpr) {
            auto argument = resolve(given, symbol.subexpression.type, function, index, depth + 1, max_depth, data, stack, gd);
            
            if (argument.error) {
//                prep(depth + gd + 1); std::cout << "   ----> false(2)!\n";
                return false; }
            args.push_back({argument});
        } else if (not are_equal_identifiers(symbol, given.symbols[index])) {
//            prep(depth + gd + 1); std::cout << "   ----> false(3)!\n";
            return false;
        }
        else index++;
    }
//    prep(depth + gd + 1); std::cout << "   ----> true!\n";
    return true;
}

static inline resolved_expression resolve_expression(const expression& given, long given_type, llvm::Function*& function, program& data, symbol_table stack, long gd);

static inline resolved_expression resolve(const expression& given, long given_type, llvm::Function*& function, long& index, long depth, long max_depth, program& data, symbol_table& stack, long gd) {
//    prep(depth + gd); std::cout << "calling resolve()\n";
    if (not given_type or depth > max_depth) { if (not given_type) abort(); return {0, {}, true}; }
    
    if (given_type == 4) { // given_type == "_2";        // temp
        if (index < (long) given.symbols.size()) {
            if (given.symbols[index].type == type::id or given.symbols[index].type == type::string) return {4, {}, false, nullptr, expression {{given.symbols[index++]}, 4}};
            else if (given.symbols[index].type == type::subexpr) return {4, {}, false, nullptr, given.symbols[index++].subexpression};
        } else abort();
    }

    long saved = index;
    for (auto s : stack.top()) {
        index = saved;
        std::vector<resolved_expression> args = {};
//        prep(depth + gd + 1); std::cout << "[resolve]: trying to match: " << expression_to_string(stack.get(s), stack) << "\n\n";
        if (matches(given, stack.get(s), given_type, args, function, index, depth, max_depth, data, stack, gd)) return {s, args};
    }
    if (index < (long) given.symbols.size() and given.symbols[index].type == type::subexpr) {
//        prep(depth + gd + 1); std::cout << "[resolve]: found subexpression...\n";
        auto resolved = resolve_expression(given.symbols[index].subexpression, given_type, function, data, stack, gd + 2);
        index++;
        return resolved;
    }
//    prep(depth + gd + 1); std::cout << "[resolve]: failing, ran out of signatures...\n";
    return {0, {}, true};
}

static inline resolved_expression resolve_expression(const expression& given, long given_type, llvm::Function*& function, program& data, symbol_table stack, long gd) {
//    prep(gd); std::cout << "calling resolve expression()\n";
    resolved_expression solution {};
    long pointer = 0;
    for (long max_depth = 0; max_depth <= max_expression_depth; max_depth++) {
        pointer = 0;
//        prep(gd + 1); std::cout << "------ trying depth = " << max_depth << " --------------\n";
        solution = resolve(given, given_type, function, pointer, 0, max_depth, data, stack, gd + 1);
        if (not solution.error and pointer == (long) given.symbols.size()) break;
    }
    if (pointer < (long) given.symbols.size()) solution.error = true;
    if (solution.error) {
        const auto t = pointer < (long) given.symbols.size() ? given.symbols[pointer].literal : given.start;
        printf("n3zqx2l: %s:%ld:%ld: error: unresolved %s @ %ld : %s ~ %s\n", data.file.name, t.line, t.column, expression_to_string(given, stack, pointer, pointer + 1).c_str(), pointer, expression_to_string(given, stack).c_str(), resolved_expression_to_string(solution, stack).c_str());
    }
    return solution;
}

static inline void debug(symbol_table stack, bool show_llvm) {
      std::cout << "\n\n---- debugging stack: ----\n";
      std::cout << "printing frames: \n";
      for (auto i = 0; i < (long) stack.frames.size(); i++) {
          std::cout << "\t ----- FRAME # "<< i <<"---- \n\t\tidxs: { ";
          for (auto index : stack.frames[i]) {
              std::cout << index << " ";
          } std::cout << "}\n";
      }
      std::cout << "\nmaster: {\n";
      auto j = 0;
      for (auto entry : stack.master) {
          std::cout << "\t" << std::setw(6) << j << ": ";
          std::cout << expression_to_string(entry.signature, stack, 0) << "\n\n";
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

static inline const char* convert_token_type_representation(enum type type) {
    switch (type) {
        case type::none: return "{null}";
        case type::string: return "string";
        case type::id: return "identifier";
        case type::op: return "operator";
        case type::subexpr: return "subexpr";
    }
}

static inline void print_lex(const std::vector<struct token>& tokens) {
    std::cout << "::::::::::LEX:::::::::::" << std::endl;
    for (auto token : tokens) {
        std::cout << "TOKEN(type: " << convert_token_type_representation(token.type) << ", value: \"" << (token.value != "\n" ? token.value : "\\n") << "\", [" << token.line << ":" << token.column << "])" << std::endl;
    }
    std::cout << ":::::::END OF LEX:::::::" << std::endl;
}

static inline void debug_token_stream(const file& file) {
    std::vector<struct token> tokens = {};
    struct token t = {};
    lexing_state state = {0, type::none, 1, 1};
    while ((t = next(file, state)).type != type::none) tokens.push_back(t);
    print_lex(tokens);
}

static inline void print_expression(expression e, int d);

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
    if (expression.llvm_type) { prep(d); std::cout << "llvmtype: " << expression.llvm_type << "\n"; }
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

static inline void print_resolved_expr(resolved_expression expr, long depth, symbol_table& stack) {
    prep(depth); std::cout << "[error = " << std::boolalpha << expr.error << "]\n";
    prep(depth); std::cout << "index = " << expr.index << " :: " << expression_to_string(stack.get(expr.index), stack, 0);
    std::cout << "\n";
    
    if (expr.expr.symbols.size()) { prep(depth); std::cout << "new signature = \n"; print_expression(expr.expr, depth + 1); std::cout << "\n"; }
    
    long i = 0;
    for (auto arg : expr.args) {
        prep(depth + 1); std::cout << "argument #" << i++ << ": \n";
        print_resolved_expr(arg, depth + 2, stack);
        prep(depth); std::cout << "\n";
    }
}


static inline resolved_expression resolve_file(const expression& given, const file& file) {
    if (given.error) return {0, {}, true};
    
    symbol_table stack;
    stack.define({{{symbol {type::id, {}, {type::id, "_"} } }, 0}}); // THE SEED: its neccessary: its the type of a program.
    stack.define({{{symbol {type::id, {}, {type::id, "_0"} } }, 1}}); // debug
    stack.define({{{symbol {type::id, {}, {type::id, "_1"} } }, 1}}); // debug
    stack.define({{{symbol {type::id, {}, {type::id, "_2"} } }, 1}}); // debug
    stack.define({{{symbol {type::id, {}, {type::id, "_id"} }, symbol {type::subexpr, {{}, 4}, {}}}, 1}}); // debug
    stack.define({{{symbol {type::id, {}, {type::id, "_eval"} }, symbol {type::subexpr, {{}, 3}, {}}}, 1}}); // debug
    stack.define({{{symbol {type::id, {}, {type::id, "_define"} }, symbol {type::subexpr, {{}, 4}, {}}, symbol {type::subexpr, {{}, 3}, {}}}, 1}}); // debug
    
    
    auto resolved = resolve_expression(given, 1, main, data, stack, 0); // what does this function call really need?
    
    printf("\n\n\n");
    print_resolved_expr(resolved, 0, stack);
    printf("\n\n\n");
    debug(stack);
    printf("\n\n\n");
        
    return resolved;
}


static inline std::unique_ptr<llvm::Module> generate(const expression& given, const file& file, llvm::LLVMContext& context) {
    if (given.error) return nullptr;
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple()); std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {});
    module->setDataLayout(target_machine->createDataLayout());
    llvm::IRBuilder<> builder(context);
    program data {file, module.get(), builder}; symbol_table stack;
    stack.define({{{symbol {type::id, {}, {type::id, "_"} } }, 0}}); // THE SEED: its neccessary: its the type of a program.
    stack.define({{{symbol {type::id, {}, {type::id, "_0"} } }, 1}}); // debug
    stack.define({{{symbol {type::id, {}, {type::id, "_1"} } }, 1}}); // debug
    stack.define({{{symbol {type::id, {}, {type::id, "_2"} } }, 1}}); // debug
    stack.define({{{symbol {type::id, {}, {type::id, "_id"} }, symbol {type::subexpr, {{}, 4}, {}}}, 1}}); // debug
    stack.define({{{symbol {type::id, {}, {type::id, "_eval"} }, symbol {type::subexpr, {{}, 3}, {}}}, 1}}); // debug
    stack.define({{{symbol {type::id, {}, {type::id, "_define"} }, symbol {type::subexpr, {{}, 4}, {}}, symbol {type::subexpr, {{}, 3}, {}}}, 1}}); // debug
    
    auto main = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(context), {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()}, false), llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main));
    auto resolved = resolve_expression(given, 1, main, data, stack, 0); // what does this function call really need?
    builder.SetInsertPoint(&main->getBasicBlockList().back());
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
    
    printf("\n\n\n");
    print_resolved_expr(resolved, 0, stack);
    printf("\n\n\n");
    debug(stack);
    printf("\n\n\n");
    std::cout << "generating code....:\n";
    module->print(llvm::outs(), nullptr);

    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) { printf("llvm: %s: error: %s\n", file.name, errors.c_str()); return nullptr; }
    else if (resolved.error) return nullptr;
    return module;
}

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
        else if (word == "-nothing" and i + 1 < argc) { args.output = output_type::nothing; }
        else if (word == "-d" and i + 1 < argc) { auto n = atoi(argv[++i]); max_expression_depth = n ? n : 4; }
        else if (word == "-!") { break; /*the linker argumnets start here.*/ }
        else if (word == "--") { break; /*the interpreter argumnets start here.*/ }
        else if (word[0] == '-') { printf("n: error: bad option: %s\n", argv[i]); exit(1); }
        else {
            const char* ext = strrchr(argv[i], '.');
            std::string extension = ext ? ext + 1 : "";
            
            if (extension == "n") {
                std::ifstream stream {argv[i]};
                if (stream.good()) {
                    std::string text {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
                    stream.close();
                    args.files.push_back({argv[i], text});
                } else { printf("n: error: unable to open \"%s\": %s\n", argv[i], strerror(errno)); exit(1); }
            } else if (extension == "ll") {
                args.files.push_back({argv[i], ""});
            } else {
                printf("n: error: received file with unknown file type. aborting...\n");
                abort();
            }
        }
    } if (args.files.empty()) { printf("n: error: no input files\n"); exit(1); } else return args;
}

static inline std::vector<std::unique_ptr<llvm::Module>> frontend(const arguments& arguments, llvm::LLVMContext& context) {
    llvm::InitializeAllTargetInfos(); llvm::InitializeAllTargets(); llvm::InitializeAllTargetMCs(); llvm::InitializeAllAsmParsers(); llvm::InitializeAllAsmPrinters();
    std::vector<std::unique_ptr<llvm::Module>> modules = {};
    for (auto file : arguments.files) {
                          
        const char* ext = strrchr(file.name, '.');
        std::string extension = ext ? ext + 1 : "";
        
        if (extension == "n") {
            lexing_state state {0, type::none, 1, 1};
            auto saved = state; // debug
            debug_token_stream(file);  // debug
            print_expression(parse(file, state, 0), 1);  // debug
            state = saved;  // debug
            modules.push_back(resolve_file(parse(file, state, 0), file, context));
            
        } else if (extension == "ll") {
            llvm::SMDiagnostic errors;
            auto module = llvm::parseAssemblyFile(file.name, errors, context);
            if (module == nullptr) {
                errors.print((std::string("llvm: ") + std::string(file.name)).c_str(), llvm::errs());
                abort();
            }
            
            module->setTargetTriple(llvm::sys::getDefaultTargetTriple()); std::string lookup_error = "";
            auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {});
            module->setDataLayout(target_machine->createDataLayout());                        
            
            std::cout << "received ll file. parsing as string into its own module. here it is: \n";
            module->print(llvm::outs(), nullptr);
        
            modules.push_back(std::move(module));
        } else {
            printf("n: error: file \"%s\" with extension \"%s\"", file.name, extension.c_str());
            abort();
        }
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
    if (args.output == output_type::nothing) {/* do nothing. */}
    else if (args.output == output_type::run) interpret(std::move(module), args);
    else if (args.output == output_type::llvm) generate_ll_file(std::move(module), args);
    else if (args.output == output_type::assembly) generate_file(std::move(module), args, llvm::TargetMachine::CGFT_AssemblyFile);
    else if (args.output == output_type::object) generate_file(std::move(module), args, llvm::TargetMachine::CGFT_ObjectFile);
    else if (args.output == output_type::exec) emit_executable(generate_file(std::move(module), args, llvm::TargetMachine::CGFT_ObjectFile), args.name);
}
int main(const int argc, const char** argv) {
    llvm::LLVMContext context;
    const auto args = get_arguments(argc, argv);
    output(args, optimize(link(frontend(args, context))));
}
