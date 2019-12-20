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
enum class lex_type {none, id, string, llvm};
enum class output_type {none, llvm, assembly, object, exec};
enum class token_type {null, id, string, llvm, op};
enum class symbol_type { none, id, string, llvm, subexpr};
namespace intrinsic { enum intrinsic_index { typeless, type, infered, llvm, empty, application, abstraction, evaluate, define }; }
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
        else if (word == "-!") { break; /*the linker argumnets start here.*/ }
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
    static char k = 0; token token = {}; auto& at = lex.index; auto& state = lex.state;
    while (lex.index < (long) file.text.size()) {
        char c = file.text[at], n; n = at + 1 < (long) file.text.size() ? file.text[at + 1] : 0;
        if (is_identifier(c) and not is_identifier(n) and state == lex_type::none) { token = { token_type::id, std::string(1, c), lex.line, lex.column}; state = lex_type::none; lex.column++; at++; return token; }
        else if (c == '\"' and state == lex_type::none) { token = { token_type::string, "", lex.line, lex.column }; state = lex_type::string; }
        else if (c == '`' and state == lex_type::none) { token = { token_type::llvm, "", lex.line, lex.column }; state = lex_type::llvm; k = n; lex.column++; at++; }
        else if (is_identifier(c) and state == lex_type::none) { token = { token_type::id, std::string(1, c), lex.line, lex.column }; state = lex_type::id; }
        else if (c == '\\' and state == lex_type::string) {
            if (n == '\\') token.value += "\\";
            else if (n == '\"') token.value += "\"";
            else if (n == 'n') token.value += "\n";
            else if (n == 't') token.value += "\t";
            else { printf("n3zqx2l: error(1,1): unknown escape sequence.\n"); } ///TODO: make this use the standard error format, and include linea nd column
            lex.column++; at++;
        } else if ((c == '\"' and state == lex_type::string) or (c == '`' and state == lex_type::llvm and n == k)) { if (state == lex_type::llvm) { lex.column++; at++; } state = lex_type::none; lex.column++; at++; return token; }
        else if (is_identifier(c) and not is_identifier(n) and state == lex_type::id) { token.value += c; state = lex_type::none; lex.column++; at++; return token; }
        else if (state == lex_type::string or state == lex_type::llvm or (is_identifier(c) and state == lex_type::id)) token.value += c;
        else if (not is_identifier(c) and not isspace(c) and state == lex_type::none) {
            token = { token_type::op, std::string(1, c), lex.line, lex.column };
            state = lex_type::none; lex.column++; at++; return token;
        } if (c == '\n') { lex.line++; lex.column = 1; } else lex.column++; at++;
    } if (state == lex_type::string) printf("n3zqx2l: %s:%ld:%ld: error: unterminated string\n", file.name, lex.line, lex.column);
    else if (state == lex_type::llvm) printf("n3zqx2l: %s:%ld:%ld: error: unterminated llvm string\n", file.name, lex.line, lex.column);
    return { token_type::null, "", lex.line, lex.column };
}





#define prep(x)   for (long i = 0; i < x; i++) std::cout << ".   "



static inline void print_llvm_table(const llvm::ValueSymbolTable& llvm_table) {
    std::cout << "-------------- printing new frame: ------------\n";
    for (auto& entry : llvm_table) {
        std::string key = entry.getKey();
        llvm::Value* value = entry.getValue();
        std::cout << "key: \"" << key << "\" == value : \n";
        value->print(llvm::outs());
        std::cout << "\n\n";
    }
}




static inline expression parse(const file& file, lexing_state& state, long d);
static inline symbol parse_symbol(const file& file, lexing_state& state, long d) {
//    prep(d); std::cout << "calling parse_symbol(): \n";
    auto saved = state;
    auto open = next(file, state);
    if (is_open_paren(open)) {
//        prep(d+1); std::cout << "is an open paren!\n";
        if (auto e = parse(file, state, d+2); not e.error) {
            auto close = next(file, state);
            if (is_close_paren(close)) return {symbol_type::subexpr, e};
            else { state = saved; printf("n3zqx2l: %s:%ld:%ld: expected \")\"\n", file.name, close.line, close.column); return {symbol_type::subexpr, e, {}, true};}
        } else state = saved;
    } else state = saved;
    auto t = next(file, state);
    if (t.type == token_type::string) return {symbol_type::string, {}, t};
    if (t.type == token_type::llvm) return {symbol_type::llvm, {}, t};
    if (t.type == token_type::id or (t.type == token_type::op and not is_open_paren(t) and not is_close_paren(t))) { ///TODO: simplify me!
//        prep(d+1); std::cout << "is an identfier!\n";
        return {symbol_type::id, {}, t}; }
    else {
        state = saved;
//        prep(d+1); std::cout << "failed to parse symbol...\n";
        return {symbol_type::none, {}, {}, true}; }
}

static inline expression parse(const file& file, lexing_state& state, long d) {
//    prep(d); std::cout << "calling parse(): \n";
    std::vector<symbol> symbols = {};
    auto saved = state; auto start = next(file, state); state = saved;
    auto symbol = parse_symbol(file, state,d+1);
    while (not symbol.error ) {
//        prep(d+1); std::cout << "in while loop of parse()...\n";
        symbols.push_back(symbol);
        saved = state;
        symbol = parse_symbol(file, state, d+2);
    } state = saved;
    if (symbol.type == symbol_type::subexpr) return {{}, 0, {}, true};
    expression result = {symbols};
    result.start = start;
    return result;
}



struct entry {
    expression signature = {};
//    resolved_expression definition = {};
    llvm::Value* value = nullptr;
    llvm::Function* function = nullptr;
    llvm::Type* llvm_type = nullptr;
};


static inline std::string expression_to_string(const expression& given, std::vector<entry> master, long begin = 0, long end = -1);
static inline expression string_to_expression(std::string given, std::vector<entry> master);


static inline void parse_ll_file(const program_data &data, const file &file) { ///TODO: temp
    llvm::SMDiagnostic function_errors; llvm::ModuleSummaryIndex my_index(true);
    llvm::MemoryBufferRef reference(file.text, file.name);
    if (not llvm::parseAssemblyInto(reference, data.module, &my_index, function_errors)) {
        printf("llvm parse assembly into:  success!\n");
    } else {
        function_errors.print("llvm: ", llvm::errs());
    }
}
static inline file open_ll_file(const char *core_name) { ///TODO: temp
    std::ifstream stream {core_name};
    if (stream.good()) {
        std::string text {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        stream.close();
        return {core_name, text};
    } else { printf("n: error: unable to open \"%s\": %s\n", core_name, strerror(errno)); exit(1); }
    return {};
}


struct symbol_table {
    
    
    
    std::vector<entry> master = {{}};
    std::vector<std::vector<long>> frames = {{}};
    program_data& data;
    
    
    void debug() {
        std::cout << "\n\n---- debugging stack: ----\n";
        
        std::cout << "printing frames: \n";
        for (auto i = 0; i < (long) frames.size(); i++) {
            std::cout << "\t ----- FRAME # "<<i<<"---- \n\t\tidxs: { ";
            for (auto index : frames[i]) {
                std::cout << index << " ";
            }
            std::cout << "}\n";
        }
        
        std::cout << "\nmaster: {\n";
        auto j = 0;
        for (auto entry : master) {
            std::cout << "\t" << std::setw(6) << j << ": ";
            std::cout << expression_to_string(entry.signature, master, 0) << "\n";
            
            if (entry.value) {
                std::cout << "\tLLVM value: \n";
                entry.value->print(llvm::errs());
            }
            if (entry.function) {
                std::cout << "\tLLVM function: \n";
                entry.function->print(llvm::errs());
            }
            j++;
        }
        std::cout << "}\n";
    }
    
    void push() { frames.push_back({frames.back()}); }
    void pop() { frames.pop_back(); }
    std::vector<long>& top() { return frames.back(); }
    expression& get(long index) { return master[index].signature; }
    long lookup(std::string key) { return std::distance(master.begin(), std::find_if(master.begin(), master.end(), [&](const entry& entry) { return key == expression_to_string(entry.signature, master, 0);})); }
    bool contains(std::string key) { return lookup(key) < (long) master.size(); }
        
    symbol id(std::string name) { return {symbol_type::id, {}, {token_type::id, name}}; }
    symbol param(long type) { return {symbol_type::subexpr, {{}, type}}; }
    
    symbol_table(program_data& data, llvm::ValueSymbolTable& llvm): data(data) {
    
//        master.push_back({/*null entry*/});
//        master.push_back({{{id("_")}, 0}});
//        master.push_back({{{id("g")}, intrinsic::type}});
//        master.push_back({{{param(intrinsic::type), param(intrinsic::type) }, intrinsic::type}});
//        //master.push_back({{{}, intrinsic::type}});
//        frames.push_back({0, 1, 2, 3});
//
        parse_ll_file(data, open_ll_file("/Users/deniylreimn/Documents/projects/n3zqx2l/examples/core.ll"));
        print_llvm_table(llvm);
    
        update(llvm);        
        debug();
        
        std::stable_sort(top().begin(), top().end(), [&](long a, long b) { return get(a).symbols.size() > get(b).symbols.size(); });
    }
    
    void update(llvm::ValueSymbolTable& llvm) {
        for (auto& entry : llvm ) {
            if (not contains(entry.getKey()))
                define({string_to_expression(entry.getKey(), master), entry.getValue()}); // fill in for function and type as well.
        }
    }
        
    void define(const entry& e) {
        std::cout << "DEFINE: (unimplemented): received: " << expression_to_string(e.signature, master) << "\n";
    }
    
};

static inline std::string resolved_expression_to_string(const resolved_expression& given, std::vector<entry> master) {
    std::string result = "(";
    long i = 0;
    for (auto symbol : master[given.index].signature.symbols) {
        if (symbol.type == symbol_type::id) result += symbol.literal.value;
        else if (symbol.type == symbol_type::subexpr) result += "(" + expression_to_string(symbol.subexpression, master) + "=" + resolved_expression_to_string(given.args[i], master) + ")";
        if (i++ < (long) master[given.index].signature.symbols.size() - 1) result += " ";
    }
    result += ")";
    if (master[given.index].signature.type) result += " " + expression_to_string(master[master[given.index].signature.type].signature, master);
    return result;
}

static inline std::string expression_to_string(const expression& given, std::vector<entry> master, long begin, long end) {
    std::string result = "(";
    long i = 0;
    for (auto symbol : given.symbols) {
        if (i < begin or (end != -1 and i >= end)) {i++; continue; }
        if (symbol.type == symbol_type::id) result += symbol.literal.value;
        else if (symbol.type == symbol_type::string) result += "\"" + symbol.literal.value + "\"";
        else if (symbol.type == symbol_type::llvm) result += "`" + symbol.literal.value + "`";
        else if (symbol.type == symbol_type::subexpr) result += "(" + expression_to_string(symbol.subexpression, master) + ")";
        if (i < (long) given.symbols.size() - 1 and not (i + 1 < begin or (end != -1 and i + 1 >= end))) result += " ";
        i++;
    }
    result += ")";
    if (given.type) result += " " + expression_to_string(master[given.type].signature, master);
    return result;
}


static inline expression resolve_type(expression e, std::vector<entry> master) {
    
    if (e.symbols.empty()) exit(1);
    
    
    if (e.symbols.size() == 1) {
        if (e.symbols.front().type == symbol_type::id) {
            
        }
    }
    
    auto signature = e.symbols.front().subexpression;
    e.symbols.erase(e.symbols.begin());
    
    // do return type:
    auto type_list = e;
    ///TODO: unimplemented.
    
    // do argument list:
    for (auto& s : signature.symbols) {
        if (s.type == symbol_type::subexpr) {
            s.subexpression = resolve_type(s.subexpression, master);
        }
    }
    return signature;
}

static inline expression string_to_expression(std::string given, std::vector<entry> master) {
    lexing_state state {0, lex_type::none, 1, 1};
    std::cout << "IN STRING TO EXPRESSION(): given = \"" << given << "\"\n";
    auto e = parse({"", given}, state, 0);
    std::cout << "STOE(): untyped: " << expression_to_string(e, master) << "\n";
    
    return resolve_type(e, master);
}

//
//static inline resolved_expression parse_llvm_type_string(const token& llvm_string, long& pointer, program_data& data) {
//    llvm::SMDiagnostic type_errors;
//    if (auto llvm_type = llvm::parseType(llvm_string.value, type_errors, *data.module)) {
//        pointer++;
//        return {intrinsic::llvm, {}, false}; // llvm_type
//    } else {
//        type_errors.print((std::string("llvm: (") + std::to_string(llvm_string.line) + "," + std::to_string(llvm_string.column) + ")").c_str(), llvm::errs());
//        return {0, {}, true};
//    }
//}
//
//static inline resolved_expression parse_llvm_string(const token& llvm_string, long& pointer, program_data& data) {
//    llvm::SMDiagnostic function_errors; llvm::ModuleSummaryIndex my_index(true);
//    llvm::MemoryBufferRef reference(llvm_string.value, data.file.name);
//    if (not llvm::parseAssemblyInto(reference, data.module, &my_index, function_errors)) {
//        pointer++;
//        return {intrinsic::llvm, {}, false}; // llvm::Type::getVoidTy(data.module->getContext())
//    } else {
//        function_errors.print((std::string("llvm: (") + std::to_string(llvm_string.line) + "," + std::to_string(llvm_string.column) + ")").c_str(), llvm::errs());
//        return {0, {}, true};
//    }
//}

static inline resolved_expression resolve(const expression& given, long given_type, llvm::Function*& function, long& index, long depth, long max_depth, program_data& data, symbol_table& stack, long gd);

static inline bool matches(const expression& given, const expression& signature, long given_type, std::vector<resolved_expression>& args, llvm::Function*& function, long& index, long depth, long max_depth, program_data& data, symbol_table stack, long gd) {
    
//    prep(depth + gd); std::cout << "calling matches(" << expression_to_string(given, stack.master) << "," << expression_to_string(signature, stack.master)<<") ...\n";
    if (given_type != signature.type) {
//        prep(depth + gd + 1); std::cout << "   ----> false(0)!\n";
        return false; }
    for (auto symbol : signature.symbols) {
        if (index >= (long) given.symbols.size()) { // this line of code might be why we cant do empty signatures?
//            prep(depth + gd + 1); std::cout << "   ----> false(1)!\n";
            return false; }
        if (symbol.type == symbol_type::subexpr) {
            auto argument = resolve(given, symbol.subexpression.type, function, index, depth + 1, max_depth, data, stack, gd);
            if (argument.error) {
//                prep(depth + gd + 1); std::cout << "   ----> false(2)!\n";
                return false; }
            args.push_back({argument});
        } else if (not are_equal_identifiers(symbol, given.symbols[index])) {
//            prep(depth + gd + 1); std::cout << "   ----> false(3)!\n";
            return false; }
        else index++;
    }
//    prep(depth + gd + 1); std::cout << "   ----> true!\n";
    return true;
}

static inline resolved_expression resolve_expression(const expression& given, long given_type, llvm::Function*& function, program_data& data, symbol_table stack, long gd);

static inline resolved_expression resolve(const expression& given, long given_type, llvm::Function*& function, long& index, long depth, long max_depth, program_data& data, symbol_table& stack, long gd) {
//    prep(depth + gd); std::cout << "calling resolve()\n";
    if (not given_type or depth > max_depth) return {0, {}, true};
    
//    else if (index < (long) given.symbols.size()
//             and given.symbols[index].type == symbol_type::llvm
//             and given_type == intrinsic::llvm)
//        return parse_llvm_string(given.symbols[index].literal, index, data);
//
//    else if (index < (long) given.symbols.size()
//             and given.symbols[index].type == symbol_type::llvm
//             and given_type == intrinsic::type)
//        return parse_llvm_type_string(given.symbols[index].literal, index, data);
//
    long saved = index;
    for (auto s : stack.top()) {
        index = saved;
        std::vector<resolved_expression> args = {};
//        prep(depth + gd + 1); std::cout << "[resolve]: trying to match: " << expression_to_string(stack.get(s), stack.master) << "\n\n";
        if (matches(given, stack.get(s), given_type, args, function, index, depth, max_depth, data, stack, gd)) return {s, args};
    }
    if (index < (long) given.symbols.size() and given.symbols[index].type == symbol_type::subexpr) {
//        prep(depth + gd + 1); std::cout << "[resolve]: found subexpression...\n";
        auto resolved = resolve_expression(given.symbols[index].subexpression, given_type, function, data, stack, gd + 2);
        index++;
        return resolved;
    }
//    prep(depth + gd + 1); std::cout << "[resolve]: failing, ran out of signatures...\n";
    return {0, {}, true};
}

static inline resolved_expression resolve_expression(const expression& given, long given_type, llvm::Function*& function, program_data& data, symbol_table stack, long gd) {
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
        printf("n3zqx2l: %s:%ld:%ld: error: unresolved %s @ %ld : %s ~ %s\n", data.file.name, t.line, t.column, expression_to_string(given, stack.master, pointer, pointer + 1).c_str(), pointer, expression_to_string(given, stack.master).c_str(), resolved_expression_to_string(solution, stack.master).c_str());
    }
    return solution;
}




void debug_arguments(const arguments& args) {
    std::cout << "file count = " <<  args.files.size() << "\n";
    for (auto a : args.files) {
        std::cout << "file: " << a.name << "\n";
        std::cout << "data: \n:::" << a.text << ":::\n";
    }
    std::cout << "exec name = " << args.name << std::endl;
}


const char* convert_token_type_representation(enum token_type type) {
    switch (type) {
        case token_type::null: return "{null}";
        case token_type::string: return "string";
        case token_type::id: return "identifier";
        case token_type::op: return "operator";
        case token_type::llvm: return "llvm";
    }
}

void print_lex(const std::vector<struct token>& tokens) {
    std::cout << "::::::::::LEX:::::::::::" << std::endl;
    for (auto token : tokens) {
        std::cout << "TOKEN(type: " << convert_token_type_representation(token.type) << ", value: \"" << (token.value != "\n" ? token.value : "\\n") << "\", [" << token.line << ":" << token.column << "])" << std::endl;
    }
    std::cout << ":::::::END OF LEX:::::::" << std::endl;
}

void debug_token_stream(const file& file) {
    std::vector<struct token> tokens = {};
    struct token t = {};
    lexing_state state = {0, lex_type::none, 1, 1};
    while ((t = next(file, state)).type != token_type::null) tokens.push_back(t);
    print_lex(tokens);
}

void print_expression(expression s, int d);

void print_symbol(symbol symbol, int d) {
    prep(d); std::cout << "symbol: \n";
    switch (symbol.type) {

        case symbol_type::id:
            prep(d); std::cout << convert_token_type_representation(symbol.literal.type) << ": " << symbol.literal.value << "\n";
            break;

        case symbol_type::llvm:
            prep(d); std::cout << "llvm literal: \'" << symbol.literal.value << "\'\n";
            break;

        case symbol_type::string:
            prep(d); std::cout << "string literal: \"" << symbol.literal.value << "\"\n";
            break;
            
        case symbol_type::subexpr:
            prep(d); std::cout << "list symbol\n";
            print_expression(symbol.subexpression, d+1);
            break;
        
        case symbol_type::none:
            prep(d); std::cout << "{NO SYMBOL TYPE}\n";
            break;
        default: break;
    }
}

void print_expression(expression expression, int d) {
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

void print_translation_unit(expression unit, const file& file) {
    std::cout << "translation unit: (" << file.name << ")\n";
    print_expression(unit, 1);
}

std::string convert_symbol_type(enum symbol_type type) {
    switch (type) {
        case symbol_type::none:
            return "{none}";
        case symbol_type::subexpr :
            return "subexpression";
        case symbol_type::string:
            return "string literal";
        case symbol_type::llvm:
            return "llvm literal";
        case symbol_type::id:
            return "identifier";
    }
}



static inline void print_resolved_expr(resolved_expression expr, long depth, symbol_table& stack) {
    prep(depth); std::cout << "[error = " << std::boolalpha << expr.error << "]\n";
    prep(depth); std::cout << "index = " << expr.index << " :: " << expression_to_string(stack.get(expr.index), stack.master, 0);
    
//    if (expr.signature.symbols.size()) {
//        std::cout << " ::: " << expression_to_string(expr.signature, stack.master);
//    }
    
    std::cout << "\n";
    
//    if (expr.llvm_type) { prep(depth); std::cout << "llvm type = "; expr.llvm_type->print(llvm::errs()); }
//    std::cout << "\n";
    long i = 0;
    for (auto arg : expr.args) {
        prep(depth + 1); std::cout << "argument #" << i++ << ": \n";
        print_resolved_expr(arg, depth + 2, stack);
        prep(depth); std::cout << "\n";
    }
}


static inline void debug_table(symbol_table table) {
    std::cout << "\n\n---- debugging stack: ----\n";
    
    std::cout << "printing frames: \n";
    for (auto i = 0; i < (long) table.frames.size(); i++) {
        std::cout << "\t ----- FRAME # "<<i<<"---- \n\t\tidxs: { ";
        for (auto index : table.frames[i]) {
            std::cout << index << " ";
        }
        std::cout << "}\n";
    }
    
    std::cout << "\nmaster: {\n";
    auto j = 0;
    for (auto entry : table.master) {
        std::cout << "\t" << std::setw(6) << j << ": ";
        std::cout << expression_to_string(entry.signature, table.master, 0) << "\n";
        
        if (entry.value) {
            std::cout << "\tLLVM value: \n";
            entry.value->print(llvm::errs());
        }
        if (entry.function) {
            std::cout << "\tLLVM function: \n";
            entry.function->print(llvm::errs());
        }
        j++;
    }
    std::cout << "}\n";
}






















static inline std::unique_ptr<llvm::Module> generate(expression program, const file& file, llvm::LLVMContext& context) {
    if (program.error) return nullptr;
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple()); std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {});
    module->setDataLayout(target_machine->createDataLayout());
    llvm::IRBuilder<> builder(context);  // if (program.symbols.empty()) return module;
    auto main = llvm::Function::Create(llvm::FunctionType::get(llvm::Type::getInt32Ty(context), {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()}, false), llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main));
    
    program_data data {file, module.get(), builder};
    symbol_table stack {data, module->getValueSymbolTable()};
    
//    for (auto& rgr : stack.back()) {
//        auto v = rgr.getValue();
//        auto f = v->getValueID();
//        std::string b = rgr.getKey();
//        std::cout << std::boolalpha;
//        std::cout << "function: " << b << " :: " << (f == llvm::Value::FunctionVal) << "\n";
//        std::cout << "global: " << b << " :: " << (f == llvm::Value::GlobalVariableVal) << "\n";
//        std::cout << "other: " << b << " :: " << (f) << "\n";
//    }
//
//    std::cout << "\n printing types:: \n";
//    auto wef = module->getIdentifiedStructTypes();
//    for (auto effe : wef) {
//        effe->print(llvm::outs());
//        std::cout << "\n";
//    }
//
//    auto fwef  = module->getTypeByName("(_)");
//    fwef->print(llvm::outs());
//
        
//    std::cout << expression_to_string(program, stack.master, 2);
//
//    exit(1);
    
    auto resolved = resolve_expression(program, intrinsic::type, main, data, stack, 0);
    
//    printf("\n\n\n");
//    print_resolved_expr(resolved, 0, stack);
//    debug_table(stack);
        
    builder.SetInsertPoint(&main->getBasicBlockList().back());
    builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0));
    
//    std::cout << "\n\n\n\ngenerating code....:\n";
//    module->print(llvm::outs(), nullptr); // debug.
    
    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) { printf("llvm: %s: error: %s\n", file.name, errors.c_str()); return nullptr; }
    else if (resolved.error) return nullptr;
    return module;
}


static inline std::vector<std::unique_ptr<llvm::Module>> frontend(const arguments& arguments, llvm::LLVMContext& context) {
    llvm::InitializeAllTargetInfos(); llvm::InitializeAllTargets(); llvm::InitializeAllTargetMCs(); llvm::InitializeAllAsmParsers(); llvm::InitializeAllAsmPrinters();
    std::vector<std::unique_ptr<llvm::Module>> modules = {};
    for (auto file : arguments.files) {
        lexing_state state {0, lex_type::none, 1, 1};
        
//        auto saved = state;
//        debug_token_stream(file);
//        print_translation_unit(parse(file, state, 0), file);
//        state = saved;
//
        modules.push_back(generate(parse(file, state, 0), file, context));
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
    if (args.output == output_type::none) interpret(std::move(module), args);
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


