/// nostril: a n3zqx2l compiler.

#include "llvm/AsmParser/Parser.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#define prep(_level)            for (nat i = _level; i--;) std::cout << ".   "
#define clear_and_return()      auto result = current; current = {}; return result;
#define revert_and_return()     revert(saved); return {true, true, true}

using nat = int_fast64_t;
using stack_frame = std::vector<nat>;
using llvm_module = std::unique_ptr<llvm::Module>;
using llvm_modules = std::vector<llvm_module>;

struct expression;
struct symbol;
struct symbol_table;

enum class output_type {none, llvm, assembly, object_file, executable};
enum class token_type {null, string, identifier, character, llvm, keyword, operator_};
enum class lex_state {none, string, string_expression, identifier, llvm_string, comment, multiline_comment};
enum class symbol_type { none, subexpression, string_literal, llvm_literal, identifier };
namespace intrin { enum intrin_name_index { typeless, type, infered, none, unit, unit_value, llvm, application, abstraction, define }; }

struct file {
    const char* name = "";
    std::string text = "";
    bool contains_main = false;
};

struct arguments {
    std::vector<file> files = {};
    enum output_type output = output_type::none;
    const char* name = "";
    bool includes_standard_library = true;
};

struct token {
    token_type type = token_type::null;
    std::string value = "";
    nat line = 0;
    nat column = 0;
};

struct saved_state {
    nat saved_c = 0;
    nat saved_line = 0;
    nat saved_column = 0;
    lex_state saved_state = lex_state::none;
    token saved_current = {};
};

struct string_literal {
    token literal = {};
    bool error = 0;
    string_literal(){}
    string_literal(bool e, bool _, bool __): error(e or _ or __) {}
    string_literal(const token& t): literal(t) {}
};

struct llvm_literal {
    token literal = {};
    bool error = 0;
    llvm_literal(){}
    llvm_literal(bool e, bool _, bool __): error(e or _ or __) {}
    llvm_literal(const token& t): literal(t) {}
};

struct identifier {
    token name = {};
    bool error = 0;
    identifier(){}
    identifier(bool e, bool _, bool __): error(e or _ or __) {}
    identifier(const token& n): name(n) {}
    identifier(const std::string& given_name) {
        name.value = given_name;
        name.type = token_type::identifier;
    }
};

struct expression {
    std::vector<symbol> symbols = {};
    nat type = 0;
    bool error = false;
    struct token start = {};
    expression() {}
    expression(bool e, bool _, bool __): error(e or _ or __) {}
    expression(const std::vector<symbol>& s, enum intrin::intrin_name_index t = intrin::typeless): symbols(s), type(t) {}
    expression(nat t): type(t) {}
};

struct symbol {
    symbol_type type = symbol_type::none;
    expression subexpression = {};
    string_literal string = {};
    llvm_literal llvm = {};
    identifier identifier = {};
    bool error = false;
    
    symbol(){}
    symbol(bool e, bool _, bool __): error(e or _ or __) {}
    symbol(symbol_type t): type(t) {}
    symbol(const string_literal& literal): type(symbol_type::string_literal), string(literal) {}
    symbol(const llvm_literal& literal): type(symbol_type::llvm_literal), llvm(literal) {}
    symbol(struct identifier id): type(symbol_type::identifier), identifier(id) {}
    symbol(const expression& e): type(symbol_type::subexpression), subexpression(e) {}
};

struct program_data {
    file file;
    llvm::Module* module;
    llvm::IRBuilder<>& builder;
};

struct resolve_state {
    symbol_table& stack;
    program_data& data;
};

struct resolved_expression {
    nat index = 0;
    std::vector<resolved_expression> args = {};
    bool error = false;
    llvm::Type* llvm_type = nullptr;
    expression signature = {};
    llvm::Value* constant = {};
};

struct signature_entry {
    expression signature = {};
    expression definition = {};
    nat precedence = 0;
    llvm::Value* value = nullptr;
    llvm::Function* function = nullptr;
    llvm::Type* llvm_type = nullptr;
};

struct symbol_table {
    std::vector<signature_entry> master = {};
    std::vector<stack_frame> frames = {};
    struct program_data& data;
    
    symbol_table(program_data& data, const std::vector<expression>& builtins);
    void update(llvm::ValueSymbolTable& llvm);
    void push_new_frame();
    void pop_last_frame();
    std::vector<nat>& top();
    expression& get(nat index);
    void define(const expression& signature, const expression& definition, nat back_from, nat parent = 0);
    void sort_top_by_largest();
    std::vector<std::string> llvm_key_symbols_in_table(llvm::ValueSymbolTable llvm);
};

// constants:
static expression failure = {true, true, true};
static expression infered_type = {{{{"__"}}}, intrin::typeless};
static expression type_type = {{{{"_"}}}, intrin::typeless};
static expression none_type = {{{{"_0"}}}, intrin::type};
static expression unit_type = {{{{"_1"}}}, intrin::type};
static expression unit_value = {{}, intrin::unit};
static expression llvm_type = {{{{"_llvm"}}}, intrin::typeless}; // placeholder
static expression application_type = {{{{"_a"}}}, intrin::type};
static expression abstraction_type = {{{{"_b"}}}, intrin::type};
static expression define_abstraction = {{{{"_c"}}, {{intrin::abstraction}}, {{intrin::type}}, {{intrin::application}}, {{intrin::application}}}, intrin::unit};

static expression chain = {
    {
        {{intrin::unit}}, {{intrin::unit}},
    }, intrin::unit
};

static expression hello_test = {
    {
        {{"hello"}}, //{{"there"}}, {{intrin::unit}}, {{"+"}}
    }, intrin::unit
};


static const std::vector<expression> builtins = {
    type_type, infered_type, none_type, unit_type, unit_value, llvm_type,
    application_type, abstraction_type, define_abstraction,
    
    hello_test, chain
};

static const resolved_expression resolution_failure = {0, {}, true};
static const resolved_expression resolved_unit_value = {intrin::unit_value, {}, false};

// global parameters:
static nat max_expression_depth = 5;
static bool debug = false;

// lexer globals:
static std::string text = "";
static const char* filename = "";

static nat c = 0;
static nat line = 0;
static nat column = 0;
static lex_state lex_state = lex_state::none;
static token current = {};

// prototypes:
void print_expression(expression e, nat d);
expression parse_expression(const file& file, bool can_be_empty);

resolved_expression resolve_expression(const expression& given, nat given_type, llvm::Function*& function, resolve_state& state);
resolved_expression search(const expression& given, nat given_type, llvm::Function*& function, nat& index, nat depth, nat max_depth, nat fdi_length, resolve_state& state);

static inline void debug_arguments(const arguments& args) {
    std::cout << "file count = " <<  args.files.size() << "\n";
    for (auto a : args.files) {
        std::cout << "file: " << a.name << "\n";
        std::cout << "data: \n:::" << a.text << ":::\n";
    }
    
    std::cout << "exec name = " << args.name << std::endl;
}

static inline arguments get_arguments(const int argc, const char** argv) {
    arguments args = {};
    for (nat i = 1; i < argc; i++) {
        const auto word = std::string(argv[i]);
        if (word == "-z") debug = true;
        else if (word == "-u") { printf("./nostril -[cdeorsuvz]\n"); exit(0); }
        else if (word == "-v") { printf("n3zqx2l: 0.0.01\nnostril: 0.0.01\n"); exit(0); }
        else if (word == "-e") args.includes_standard_library = false;
        else if (word == "-r" and i + 1 < argc) { args.output = output_type::llvm; args.name = argv[++i]; }
        else if (word == "-s" and i + 1 < argc) { args.output = output_type::assembly; args.name = argv[++i]; }
        else if (word == "-c" and i + 1 < argc) { args.output = output_type::object_file; args.name = argv[++i]; }
        else if (word == "-o" and i + 1 < argc) { args.output = output_type::executable; args.name = argv[++i]; }
        else if (word == "-d" and i + 1 < argc) { auto n = atoi(argv[++i]); max_expression_depth = n ? n : 4; }
        else if (word == "-/") { break; /*the linker argumnets start here.*/ }
        else if (word[0] == '-') { printf("bad option: %s\n", argv[i]); exit(1); }
        else {
            std::ifstream stream {argv[i]};
            if (stream.good()) {
                std::string text {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
                stream.close();
                args.files.push_back({argv[i], text});
            } else {
                printf("Unable to open \"%s\" \n", argv[i]);
                perror("open");
                exit(1);
            }
        }
    }
    if (args.files.empty()) { printf("nostril: error: no input files\n"); exit(1); }
    if (debug) debug_arguments(args);
    return args;
}

static inline bool is_identifier(char c) { return isalnum(c) or c == '_'; }
static inline bool is_operator(char c) { return (not is_identifier(c) or not isascii(c)) and c != ' ' and c != '\t' and c != '\n'; }
static inline bool is_valid(nat c) { return c >= 0 and c < (nat) text.size(); }
static inline void set_current(token_type t, enum lex_state s) { current = {t, "", line, column}; lex_state = s; }
static inline void advance_by(nat n) { for (nat i = n; i--;) if (text[c++] == '\n') { line++; column = 1; } else column++; }
static inline bool is_operator(const token& t) { return t.type == token_type::operator_; }
static inline bool is_identifier(const token& t) { return t.type == token_type::identifier; }
static inline bool is_close_paren(const token& t) { return is_operator(t) and t.value == ")"; }
static inline bool is_open_paren(const token& t) { return is_operator(t) and t.value == "("; }
static inline bool is_reserved_operator(token t) { return is_open_paren(t) or is_close_paren(t); }
static inline bool subexpression(const symbol& s) { return s.type == symbol_type::subexpression; }
static inline bool identifier(const symbol& s) { return s.type == symbol_type::identifier; }
static inline bool llvm_string(const symbol& s) { return s.type == symbol_type::llvm_literal; }
//static inline bool string_literal(const symbol& s) { return s.type == symbol_type::string_literal; }
static inline bool parameter(const symbol &symbol) { return subexpression(symbol); }
static inline bool are_equal_identifiers(const symbol &first, const symbol &second) { return identifier(first) and identifier(second) and first.identifier.name.value == second.identifier.name.value; }
static inline bool is_donothing_call(llvm::Instruction* ins) { return ins and std::string(ins->getOpcodeName()) == "call" and std::string(ins->getOperand(0)->getName()) == "llvm.donothing"; }
static inline bool is_unreachable_instruction(llvm::Instruction* ins) { return ins and std::string(ins->getOpcodeName()) == "unreachable"; }

static inline void check_for_lexing_errors() {
    if (lex_state == lex_state::string) printf("n3zqx2l: %s:%lld,%lld: error: unterminated string\n", filename, line, column);
    else if (lex_state == lex_state::llvm_string) printf("n3zqx2l: %s:%lld,%lld: error: unterminated llvm string\n", filename, line, column);
    else if (lex_state == lex_state::multiline_comment) printf("n3zqx2l: %s:%lld,%lld: error: unterminated multi-line comment\n", filename, line, column);
    if (lex_state == lex_state::string or lex_state == lex_state::llvm_string or lex_state == lex_state::multiline_comment) exit(1);
}

static inline token next() {
    while (true) {
        if (c >= (nat) text.size()) { check_for_lexing_errors(); return {token_type::null, "", line, column}; }

             if (text[c] == ';' and is_valid(c+1) and     isspace(text[c+1]) and lex_state == lex_state::none) lex_state = lex_state::comment;
        else if (text[c] == ';' and is_valid(c+1) and not isspace(text[c+1]) and lex_state == lex_state::none) lex_state = lex_state::multiline_comment;

        // ------------------- starting and finising ----------------------
        else if (is_identifier(text[c]) and is_valid(c+1) and not is_identifier(text[c+1]) and lex_state == lex_state::none) {
            set_current(token_type::identifier, lex_state::none);
            current.value = text[c];
            advance_by(1);
            clear_and_return();
        }
        
        // ---------------------- starting --------------------------
        else if (text[c] == '\"' and lex_state == lex_state::none) { set_current(token_type::string, lex_state::string); }
        else if (text[c] == '`' and lex_state == lex_state::none) { set_current(token_type::llvm, lex_state::llvm_string); }
        else if (is_identifier(text[c]) and (lex_state == lex_state::none)) {
            set_current(token_type::identifier, lex_state::identifier);
            current.value += text[c];
        }
        
        // ---------------------- escaping in strings --------------------------
        else if (text[c] == '\\' and lex_state == lex_state::string) {
            if (is_valid(c+1) and text[c+1] == '\"') { current.value += "\""; advance_by(1); }
            else if (is_valid(c+1) and text[c+1] == 'n') { current.value += "\n"; advance_by(1); }
            else if (is_valid(c+1) and text[c+1] == 't') { current.value += "\t"; advance_by(1); }
            else if (is_valid(c+1) and text[c+1] == '\\') { current.value += "\\"; advance_by(1); }
        }
        
        //---------------------- finishing  ----------------------
        else if ((text[c] == '\n' and lex_state == lex_state::comment) or (text[c] == ';' and lex_state == lex_state::multiline_comment)) lex_state = lex_state::none;
        else if ((text[c] == '\"' and lex_state == lex_state::string)  or (text[c] == '`' and lex_state == lex_state::llvm_string)) {
            lex_state = lex_state::none;
            advance_by(1);
            clear_and_return();
        
        } else if (is_identifier(text[c]) and is_valid(c+1) and not is_identifier(text[c+1]) and lex_state == lex_state::identifier) {
            current.value += text[c];
            lex_state = lex_state::none;
            advance_by(1);
            clear_and_return();

        // ---------------- pushing ----------------
        } else if (lex_state == lex_state::string or lex_state == lex_state::llvm_string or (is_identifier(text[c]) and lex_state == lex_state::identifier)) current.value += text[c];
        else if (lex_state == lex_state::comment or lex_state == lex_state::multiline_comment) {}
        else if (is_operator(text[c]) and lex_state == lex_state::none) {
            set_current(token_type::operator_, lex_state::none);
            current.value = text[c];
            advance_by(1);
            clear_and_return();
        }
        advance_by(1);
    }
}

static inline saved_state save() {
    saved_state result;
    result.saved_c = c;
    result.saved_line = line;
    result.saved_column = column;
    result.saved_state = lex_state;
    result.saved_current = current;
    return result;
}

static inline void revert(saved_state s) {
    c = s.saved_c;
    line = s.saved_line;
    column = s.saved_column;
    lex_state = s.saved_state;
    current = s.saved_current;
}

static inline void start_lex(const file& file) { // this function should be called before lexing a given file.
    text = file.text;
    filename = file.name;
    c = 0;
    line = 1;
    column = 1;
    lex_state = lex_state::none;
    current = {};
}

static inline const char* convert_token_type_representation(enum token_type type) {
    switch (type) {
        case token_type::null: return "EOF";
        case token_type::string: return "string";
        case token_type::identifier: return "identifier";
        case token_type::keyword: return "keyword";
        case token_type::operator_: return "operator";
        case token_type::character: return "character";
        case token_type::llvm: return "llvm";
    }
}

static inline void print_lex(const std::vector<struct token>& tokens) {
    std::cout << "::::::::::LEX:::::::::::" << std::endl;
    for (auto token : tokens) {
        std::cout << "TOKEN(type: " << convert_token_type_representation(token.type) << ", value: \"" << (token.value != "\n" ? token.value : "\\n") << "\", [" << token.line << ":" << token.column << "])" << std::endl;
    }
    std::cout << ":::::::END OF LEX:::::::" << std::endl;
}

static inline void debug_token_stream() {
    std::vector<struct token> tokens = {};
    struct token t = {};
    while ((t = next()).type != token_type::null) tokens.push_back(t);
    print_lex(tokens);
}

static inline void print_symbol(symbol symbol, nat d) {
    switch (symbol.type) {

        case symbol_type::identifier:
            prep(d); std::cout << convert_token_type_representation(symbol.identifier.name.type) << ": " << symbol.identifier.name.value << "\n";
            break;

        case symbol_type::llvm_literal:
            prep(d); std::cout << "llvm literal: \'" << symbol.llvm.literal.value << "\'\n";
            break;

        case symbol_type::string_literal:
            prep(d); std::cout << "string literal: \"" << symbol.string.literal.value << "\"\n";
            break;
            
        case symbol_type::subexpression:
            prep(d); std::cout << "subexpression:\n";
            print_expression(symbol.subexpression, d+1);
            break;

        case symbol_type::none:
            prep(d); std::cout << "{NO SYMBOL TYPE}\n";
            break;
        default: break;
    }
}

void print_expression(expression expression, nat d) {
    prep(d); std::cout << "expression: \n";
    prep(d); std::cout << std::boolalpha << "error: " << expression.error << "\n";
    prep(d); std::cout << "symbol count: " << expression.symbols.size() << "\n";
    prep(d); std::cout << "symbols: \n";
    nat i = 0;
    for (auto symbol : expression.symbols) {
        prep(d+1); std::cout << i << ": \n";
        print_symbol(symbol, d+1);
        std::cout << "\n";
        i++;
    }
    
    prep(d); std::cout << "type = " << expression.type << "\n";
}

static inline void print_translation_unit(expression unit, file file) {
    std::cout << "translation unit: (" << file.name << ")\n";
    print_expression(unit, 1);
}

static inline struct string_literal parse_string_literal() {
    auto saved = save();
    auto t = next();
    if (t.type != token_type::string) { revert_and_return(); }
    return {t};
}

static inline llvm_literal parse_llvm_literal() {
    auto saved = save();
    auto t = next();
    if (t.type != token_type::llvm) { revert_and_return(); }
    return {t};
}

static inline struct identifier parse_identifier() {
    auto saved = save();
    auto t = next();
    if (not is_identifier(t) and (is_reserved_operator(t) or not is_operator(t))) { revert_and_return(); }
    return {t};
}

static inline symbol parse_symbol(const file& file) {
    auto saved = save();
    auto t = next();
    if (is_open_paren(t)) {
        auto expressions = parse_expression(file, true);
        if (not expressions.error) {
            auto saved_t = t;
            t = next();
            expressions.start = t;
            if (is_close_paren(t)) return { expressions };
            else {
                printf("n3zqx2l: %s:%lld:%lld: unexpected %s, \"%s\", expected \")\" to close expression\n",
                       file.name, saved_t.line, saved_t.column, convert_token_type_representation(t.type), t.value.c_str());
                revert_and_return();
            }
        }
    } else revert(saved);

    auto string = parse_string_literal();
    if (not string.error) return string;
    else revert(saved);
    
    auto llvm = parse_llvm_literal();
    if (not llvm.error) return llvm;
    else revert(saved);

    auto identifier = parse_identifier();
    if (not identifier.error) return identifier;
    else revert(saved);

    revert_and_return();
}

expression parse_expression(const file& file, bool can_be_empty) {
            
    std::vector<symbol> symbols = {};
    auto saved = save();
    auto start = next();
    revert(saved);
    
    auto symbol = parse_symbol(file);
    while (not symbol.error) {
        symbols.push_back(symbol);
        saved = save();
        symbol = parse_symbol(file);
    }
    revert(saved);
    
    expression result = {symbols};
    result.start = start;
    result.error = not can_be_empty and symbols.empty();
    return result;
}

static inline expression parse(const file& file) {
    start_lex(file);
    if (debug) { debug_token_stream(); start_lex(file); }
    auto unit = parse_expression(file, /*can_be_empty = */true);
    if (debug) print_translation_unit(unit, file);
    return unit;
}

static inline std::string expression_to_string(const expression& given, symbol_table& stack) {
    std::string result = "(";
    nat i = 0;
    for (auto symbol : given.symbols) {
        if (symbol.type == symbol_type::identifier) result += symbol.identifier.name.value;
        else if (symbol.type == symbol_type::subexpression) result += "(" + expression_to_string(symbol.subexpression, stack) + ")";
        if (i++ < (nat) given.symbols.size() - 1) result += " ";
    }
    result += ")";
    if (given.type) result += " " + expression_to_string(stack.master[given.type].signature, stack);
    return result;
}

static inline void print_resolved_expr(resolved_expression expr, nat depth, resolve_state& state) {
    prep(depth); std::cout << "[error = " << std::boolalpha << expr.error << "]\n";
    prep(depth);
    
    std::cout << "index = " << expr.index << " :: " << expression_to_string(state.stack.get(expr.index), state.stack);
    
    if (expr.signature.symbols.size()) {
        std::cout << " ::: " << expression_to_string(expr.signature, state.stack);
    }
    
    std::cout << "\n";
    
    if (expr.llvm_type) { prep(depth); std::cout << "llvm type = "; expr.llvm_type->print(llvm::errs()); }
    std::cout << "\n";
    nat i = 0;
    for (auto arg : expr.args) {
        prep(depth + 1); std::cout << "argument #" << i++ << ": \n";
        print_resolved_expr(arg, depth + 2, state);
        prep(depth); std::cout << "\n";
    }
}

static inline void print_resolved_unit(resolved_expression unit, resolve_state& state) {
    std::cout << "---------- printing resolved tranlation unit: ------------\n\n";
    print_resolved_expr(unit, 0, state);
}

static inline void debug_table(symbol_table table) {
    std::cout << "---- debugging stack: ----\n";
    
    std::cout << "printing frames: \n";
    for (auto i = 0; i < (nat) table.frames.size(); i++) {
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
        std::cout << expression_to_string(entry.signature, table) << "\n";
        
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

static inline bool contains_final_terminator(llvm::Function* main_function) {
    auto& blocks = main_function->getBasicBlockList();
    if (blocks.size()) {
        auto& instructions = blocks.back().getInstList();
        auto& last = instructions.back();
        if (last.isTerminator()) return true;
    }
    return false;
}

static inline void append_return_0_statement(llvm::IRBuilder<> &builder, llvm::Function* main_function, llvm::LLVMContext& context) {
    llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    builder.SetInsertPoint(&main_function->getBasicBlockList().back());
    builder.CreateRet(value);
}

static inline llvm::Function* create_main(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, llvm_module& module) {
    std::vector<llvm::Type*> state = {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()};
    auto main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), state, false);
    auto main_function = llvm::Function::Create(main_type, llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main_function));
    return main_function;
}

static inline void prune_extraneous_subexpressions(expression& given) {
    while (given.symbols.size() == 1
           and subexpression(given.symbols[0])
           /*and given.symbols[0].subexpression.symbols.size()*/) {
        auto save = given.symbols[0].subexpression.symbols;
        given.symbols = save;
    }
    for (auto& symbol : given.symbols) if (subexpression(symbol)) prune_extraneous_subexpressions(symbol.subexpression);
}


static inline void verify(const file& file, llvm_module& module, resolved_expression& resolved_program) {
    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) {
        printf("llvm: %s: %s", file.name, errors.c_str());
        resolved_program.error = true;
    }
}

///important note about this function:
/// it leaves artifacts in the function after use, which must be removed:
/// any occurence of a unreachable statement which is directly preceeded by
/// a llvm.do_nothing() call, should be removed before execution of the function.

static inline bool parse_llvm_string_as_instruction(const std::string& given, llvm::Function*& original, resolve_state& state, llvm::SMDiagnostic& errors) {
    static nat num = 0;
    std::string body = "";
    original->print(llvm::raw_string_ostream(body) << "");
    body.pop_back(); // delete the newline
    body.pop_back(); // delete the close brace
    body += given + "\n call void @llvm.donothing() \n unreachable \n } \n";
    const std::string current_name = original->getName();
    original->setName("_anonymous_" + std::to_string(num++));
    llvm::MemoryBufferRef reference(body, "<llvm-string>");
    llvm::ModuleSummaryIndex my_index(true);
    if (llvm::parseAssemblyInto(reference, state.data.module, &my_index, errors)) {
        original->setName(current_name);
        return false;
    } else {
        original->getBasicBlockList().clear();
        original = state.data.module->getFunction(current_name);
        return true;
    }
}

static inline resolved_expression parse_llvm_string(llvm::Function*& function, const std::string& llvm_string, nat& pointer, resolve_state& state) {
    llvm::SMDiagnostic instruction_errors, function_errors, type_errors;
    llvm::ModuleSummaryIndex my_index(true);
    llvm::MemoryBufferRef reference(llvm_string, "<llvm-string>");
    
    if (not llvm::parseAssemblyInto(reference, state.data.module, &my_index, function_errors) or
        parse_llvm_string_as_instruction(llvm_string, function, state, instruction_errors)) {
        pointer++;
        return {intrin::unit_value, {}, false};
    } else if (auto llvm_type = llvm::parseType(llvm_string, type_errors, *state.data.module)) {
        pointer++;
        return {intrin::typeless, {}, false, llvm_type};
    } else {
        printf("llvm: "); function_errors.print(state.data.file.name, llvm::errs());
        printf("llvm: "); instruction_errors.print(state.data.file.name, llvm::errs());
        printf("llvm: "); type_errors.print(state.data.file.name, llvm::errs());
        return {0, {}, true};
    }
}

static resolved_expression construct_signature(nat fdi_length, const expression& given, nat& index) {
    resolved_expression result = {intrin::typeless};
    result.signature = std::vector<symbol>(given.symbols.begin() + index, given.symbols.begin() + index + fdi_length);
    index += fdi_length;
    return result;
}

static inline bool matches(const expression& given, const expression& signature, nat given_type, std::vector<resolved_expression>& args, llvm::Function*& function,
                           nat& index, nat depth, nat max_depth, nat fdi_length, resolve_state& state) {
    if (given_type != signature.type) return false;
    
    for (auto symbol : signature.symbols) {
        if (index >= (nat) given.symbols.size()) return false;
        if (parameter(symbol)) {
            auto argument = search(given, symbol.subexpression.type, function, index, depth + 1, max_depth, fdi_length, state);
            if (argument.error) return false;
            args.push_back({argument});
        } else if (not are_equal_identifiers(symbol, given.symbols[index])) return false;
        else index++;
    }
    return true;
}

resolved_expression search(const expression& given, nat given_type, llvm::Function*& function,
                           nat& index, nat depth, nat max_depth, nat fdi_length, resolve_state& state) {
    
    if (not given_type or depth > max_depth) return resolution_failure;
    
    else if (index < (nat) given.symbols.size() and subexpression(given.symbols[index])) {
        auto resolved = resolve_expression(given.symbols[index].subexpression, given_type, function, state);
        if (not resolved.error) index++;
        return resolved;
    }
    else if (given_type == intrin::abstraction) return construct_signature(fdi_length, given, index);
    else if (index < (nat) given.symbols.size() and llvm_string(given.symbols[index]) and given_type == intrin::unit) return parse_llvm_string(function, given.symbols[index].llvm.literal.value, index, state);
//    else if (string_literal(given.symbols[index]) and given_type == intrin::llvm)  parse_string(given, index, state);
    
    nat saved = index;
    for (auto s : state.stack.top()) {
        index = saved;
        std::vector<resolved_expression> args = {};
        if (matches(given, state.stack.get(s), given_type, args, function, index, depth, max_depth, fdi_length, state))
            return {s, args, false};
    }
    return resolution_failure;
}

resolved_expression resolve_expression(const expression& given, nat given_type, llvm::Function*& function, resolve_state& state) {
    
    resolved_expression solution {};
    nat pointer = 0;
    for (nat max_depth = 0; max_depth <= max_expression_depth; max_depth++) {
//        for (nat fdi_length = given.symbols.size(); fdi_length--;) {
            pointer = 0;
            solution = search(given, given_type, function, pointer, 0, max_depth, 0/*fdi_length*/, state);
            if (not solution.error and pointer == (nat) given.symbols.size()) break;
//        }
        if (not solution.error and pointer == (nat) given.symbols.size()) break;
    }
    
//    if (given.symbols.empty() and given_type == intrin::unit) return resolved_unit_value;
    if (pointer < (nat) given.symbols.size()) solution.error = true;
    if (solution.error) printf("n3zqx2l: %s:%lld:%lld: error: unresolved expression: %s\n", state.data.file.name, given.start.line, given.start.column, expression_to_string(given, state.stack).c_str());
    return solution;
}

static inline void delete_empty_blocks(llvm_module& module) {
    for (auto& function : module->getFunctionList()) {
        llvm::SmallVector<llvm::BasicBlock*, 100> blocks = {};
        for (auto& block : function.getBasicBlockList()) {
            if (block.empty()) blocks.push_back(&block);
        }
        llvm::DeleteDeadBlocks(blocks);
    }
}

static inline void move_lone_terminators_into_previous_blocks(llvm_module& module) {
    for (auto& function : module->getFunctionList()) {
        llvm::BasicBlock* previous = nullptr;
        
        for (auto& block : function.getBasicBlockList()) {
            auto& instructions = block.getInstList();
            
            if (previous and instructions.size() == 1
                and instructions.back().isTerminator())
                instructions.back().moveAfter(&previous->getInstList().back());
            
            previous = &block;
        }
    }
    
    ///TODO: unfinished.
    ///KNOWN BUG: when we have no terminators in sight, this function
    /// does remove the unneccessary basic block which is put between the
    /// bits of code which should be in the same block.
}

static inline void remove_donothing_remnants(llvm_module& module) {
    for (auto& function : module->getFunctionList()) {
        for (auto& block : function.getBasicBlockList()) {
            auto terminator = block.getTerminator();
            if (!terminator) continue;
            auto previous = terminator->getPrevNonDebugInstruction();
            if (is_unreachable_instruction(terminator) and is_donothing_call(previous)) {
                previous->eraseFromParent();
                terminator->eraseFromParent();
            }
        }
    }
}

void symbol_table::update(llvm::ValueSymbolTable& llvm) {
    ///TODO: unfinished.
}

void symbol_table::push_new_frame() { frames.push_back({frames.back()}); }
void symbol_table::pop_last_frame() { frames.pop_back(); }
std::vector<nat>& symbol_table::top() { return frames.back(); }
expression& symbol_table::get(nat index) { return master[index].signature; }

void symbol_table::define(const expression& signature, const expression& definition, nat back_from, nat parent) {
    ///TODO: unfinished.
    
    // this function should do a check for if the signature is already
    // defined in the current scope. if so, then simply overrite its data.
    
    frames[frames.size() - (++back_from)].push_back(master.size());
    master.push_back({signature, definition, parent});
    
    //we need to define it the LLVM symbol table!
    // and we need to define it of the right type, as well.
    sort_top_by_largest();
}

void symbol_table::sort_top_by_largest() {
    std::stable_sort(top().begin(), top().end(), [&](nat a, nat b) { return get(a).symbols.size() > get(b).symbols.size(); });
}

symbol_table::symbol_table(program_data& data, const std::vector<expression>& builtins) : data(data) {
    master.push_back({});  // the null entry. a type (index) of 0 means it has no type.
    for (auto signature : builtins) master.push_back({signature, {}, {}});
    std::vector<nat> compiler_intrinsics = {};
    for (nat i = 0; i < (nat) builtins.size(); i++) compiler_intrinsics.push_back(i + 1);
    frames.push_back({compiler_intrinsics});
    sort_top_by_largest();
}

std::vector<std::string> symbol_table::llvm_key_symbols_in_table(llvm::ValueSymbolTable llvm) {
    std::vector<std::string> result = {};
    for (auto i = llvm.begin(); i != llvm.end(); i++) result.push_back(i->getKey());
    return result;
}

static inline void set_data_for(llvm_module& module) {
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {});
    module->setDataLayout(target_machine->createDataLayout());
}

static inline llvm_module resolve(expression program, const file& file, llvm::LLVMContext& context) {
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    set_data_for(module);
    llvm::IRBuilder<> builder(context);
    program_data data {file, module.get(), builder};
    symbol_table stack {data, builtins};
    resolve_state state {stack, data};
    stack.sort_top_by_largest();
    auto main = create_main(builder, context, module);
    builder.CreateCall(llvm::Intrinsic::getDeclaration(module.get(), llvm::Intrinsic::donothing));
    prune_extraneous_subexpressions(program);
    auto resolved = resolve_expression(program, intrin::unit, main, state);
    remove_donothing_remnants(module);
    move_lone_terminators_into_previous_blocks(module);
    delete_empty_blocks(module);
    if (not contains_final_terminator(main)) append_return_0_statement(builder, main, context);
    verify(file, module, resolved);
    if (debug) {
        debug_table(state.stack);
        print_resolved_unit(resolved, state);
        module->print(llvm::outs(), nullptr);
    }
    if (resolved.error) exit(10);
    else return module;
}

static inline llvm_modules frontend(const arguments& arguments, llvm::LLVMContext& context) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    llvm_modules modules = {};
    modules.reserve(arguments.files.size());
    for (auto file : arguments.files) modules.push_back(resolve(parse(file), file, context));
    return modules;
}

static inline llvm_module link(llvm_modules&& modules) {
    if (modules.empty()) return {};
    auto result = std::move(modules.back());
    modules.pop_back();
    for (auto& module : modules) if (llvm::Linker::linkModules(*result, std::move(module))) exit(1);
    return result;
}

static inline void interpret(llvm_module module, const arguments& arguments) {
    auto jit = llvm::EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::JIT).create();
    jit->finalizeObject();
    exit(jit->runFunctionAsMain(jit->FindFunctionNamed("main"), {arguments.name}, nullptr));
}

static inline llvm_module optimize(llvm_module&& module) { return std::move(module); } ///TODO: unfinished.

static inline void generate_ll_file(llvm_module module, const arguments& arguments) {
    std::error_code error;
    llvm::raw_fd_ostream dest(std::string(arguments.name) + ".ll", error, llvm::sys::fs::F_None);
    if (error) exit(1);
    module->print(dest, nullptr);
}

static inline std::string generate_object_file(llvm_module module, const arguments& arguments) {
    std::string lookup_error = "";
    auto target_machine = llvm::TargetRegistry::lookupTarget(module->getTargetTriple(), lookup_error)->createTargetMachine(module->getTargetTriple(), "generic", "", {}, {}, {}); ///TODO: make this not generic!
    
    auto object_filename = std::string(arguments.name) + ".o";
    std::error_code error;
    llvm::raw_fd_ostream dest(object_filename, error, llvm::sys::fs::F_None);
    if (error) exit(1);
    
    llvm::legacy::PassManager pass;
    if (target_machine->addPassesToEmitFile(pass, dest, nullptr, llvm::TargetMachine::CGFT_ObjectFile)) {
        std::remove(object_filename.c_str());
        exit(1);
    }
    pass.run(*module);
    dest.flush();
    return object_filename;
}

static inline void emit_executable(const std::string& object_file, const arguments& arguments) {
    std::string link_command = "ld -macosx_version_min 10.14 -lSystem -lc -o " + std::string(arguments.name) + " " + object_file + " ";
    std::system(link_command.c_str());
    std::remove(object_file.c_str());
}

static inline void output(const arguments& args, llvm_module&& module) {
    if (args.output == output_type::none) interpret(std::move(module), args);
    else if (args.output == output_type::llvm) { generate_ll_file(std::move(module), args); }
    else if (args.output == output_type::assembly) { printf("cannot output .s file, unimplemented\n"); /*generate_s_file();*/ }
    else if (args.output == output_type::object_file) generate_object_file(std::move(module), args);
    else if (args.output == output_type::executable) emit_executable(generate_object_file(std::move(module), args), args);
}

int main(const int argc, const char** argv) {
    llvm::LLVMContext context;
    const auto args = get_arguments(argc, argv);
    output(args, optimize(link(frontend(args, context))));
}
