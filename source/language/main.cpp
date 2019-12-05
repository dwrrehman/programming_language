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
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>


///// MARKED FOR DELETION SOON.
//// DELETE ME:    this should just be inlined into logic code as simply print statements, using builtin 256 colorings.
//
//#include "error.hpp"
//
//#include "lexer.hpp"
//#include "color.h"
//#include "analysis_ds.hpp"
//#include "symbol_table.hpp"
//
//#include "llvm/Support/SourceMgr.h"
//
//#include <sstream>
//#include <iostream>
//#include <iomanip>
//
//static std::string error_heading(const std::string& filename, nat line, nat column) {
//
////    std::ostringstream s;
////    s <<
////    cBOLD cCYAN << language_name << cRESET
////    cBOLD cGRAY ": " cRESET;
////    if (filename != "")
////        s << cBOLD cMAGENTA << filename << cGRAY ":" cRESET ;
////
////    if (line and column)
////        s << cBOLD cWHITE << line << cGRAY ":" cRESET cBOLD cWHITE << column << cGRAY ":" cRESET;
////
////    s << cBOLD cBRIGHT_RED " error: " cRESET
////    cBOLD;
////    return s.str();
//    return "";
//}
//
//static std::string warning_heading(const std::string& filename, nat line, nat column) {
////    std::ostringstream s;
////    s <<
////    cBOLD cCYAN << language_name << cRESET
////    cBOLD cGRAY ": " cRESET;
////
////    if (filename != "")
////        s << cBOLD cMAGENTA << filename << cGRAY ":" cRESET ;
////    if (line and column)
////        s << cBOLD cWHITE << line << cGRAY ":" cRESET cBOLD cWHITE << column << cGRAY ":" cRESET;
////
////    s << cBOLD cBRIGHT_YELLOW " warning: " cRESET
////    cBOLD;
////    return s.str();
//    return "";
//}
//
//
//void print_error_message(const std::string& filename, const std::string& message, nat line, nat column) {
////    std::cerr << error_heading(filename, line, column) << message << cRESET << std::endl;
//}
//
//void print_warning_message(const std::string& filename, const std::string& message, nat line, nat column) {
////    std::cerr << warning_heading(filename, line, column) << message << std::endl;
//}
//
//void print_lex_error(const std::string& filename, const std::string& state_name, nat line, nat column) {
////    std::cerr << error_heading(filename, line, column) << "unterminated " << state_name << std::endl;
//}
//
//void print_parse_error(const std::string& filename, nat line, nat column, const std::string& type, std::string found, const std::string& expected) {
////    if (type == "{null}" or found == "\n" or type == "indent") {
////        if (type == "{null}") found = "end of file";
////        if (found == "\n") found = "newline";
////        if (type == "indent") found = "indent";
////        std::cerr << error_heading(filename, line, column) << "unexpected " << found << ", expected " << expected << std::endl;
////    } else std::cerr << error_heading(filename, line, column) << "unexpected " << type << ", \"" << found << "\", expected " << expected << std::endl;
//}
//
//void print_llvm_error(const llvm::SMDiagnostic& errors, state &state) {
////    std::cout << cBOLD cGRAY << "llvm: " << cRESET << std::flush;
////    errors.print(state.data.file.name.c_str(), llvm::errs());
////    std::cout << std::flush;
//}
//
//void print_unresolved_error(const expression& given, state &state) {
////    const std::string name = expression_to_string(given, state.stack);
////    print_error_message(state.data.file.name, "unresolved expression: " + name, given.starting_token.line, given.starting_token.column);
//}




#define prep(_level) for (nat i = _level; i--;) std::cout << ".   "
#define clear_and_return()  auto result = current; current = {}; return result;
#define revert_and_return()    revert(saved); return {true, true, true}

using nat = int_fast64_t;

enum class token_type {null, string, identifier, character, llvm, keyword, operator_, indent};
enum class lexing_state {none, string, string_expression, identifier, llvm_string, comment, multiline_comment, indent};

struct file {
    const char* name = "";
    std::string text = "";
    bool is_main = false;
};

struct arguments {
    std::vector<file> files = {};
    const char* executable_name = "a.out";
    bool use_repl = false;
    bool empty = false;
    bool interpret = false;
    bool error = false;
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
    lexing_state saved_state = lexing_state::none;
    token saved_current = {};
};


struct expression_list;
struct expression;
struct symbol;

enum class symbol_type { none, subexpression, string_literal, llvm_literal, identifier, newline, indent };

// literals:
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

struct expression_list {
    std::vector<expression> list = {};
    bool error = {};
    token starting_token = {};
        
    expression_list(){}
    expression_list(bool e, bool _, bool __): error(e or _ or __) {}
    expression_list(const std::vector<expression>& es): list(es) {}
    expression_list(const std::vector<expression>& es, bool e): list(es), error(e) {}
};


namespace intrin { enum intrin_name_index { typeless, type, infered, none, unit, unit_value, llvm, application, abstraction, define }; }


struct expression {
    std::vector<symbol> symbols = {};
    nat indent_level = 0;
    nat type = 0;
    bool error = false;
    struct token starting_token = {};
    
    expression() {}
    expression(bool e, bool _, bool __): error(e or _ or __) {}
    expression(const std::vector<symbol>& s, enum intrin::intrin_name_index t = intrin::typeless): symbols(s), type(t) {}
    expression(nat t): type(t) {}
};

struct symbol {
    symbol_type type = symbol_type::none;
    expression_list expressions = {};
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
    symbol(const expression_list& es): type(symbol_type::subexpression), expressions(es) {}
    symbol(const expression& e): symbol(expression_list {{e}}) {}
};

using llvm_modules = std::vector<std::unique_ptr<llvm::Module>>;
using llvm_module = std::unique_ptr<llvm::Module>;

struct program_data {
    file file;
    llvm::Module* module;
    llvm::IRBuilder<>& builder;
};

struct symbol_table;

struct state {
    symbol_table& stack;
    program_data& data;
};

struct resolved_expression_list;

struct resolved_expression {
    nat index = 0;
    std::vector<resolved_expression_list> args = {};
    bool error = false;
    llvm::Type* llvm_type = nullptr;
    expression signature = {};
    llvm::Value* constant = {};
};

struct resolved_expression_list {
    std::vector<resolved_expression> list = {};
    bool error = false;
};

using stack_frame = std::vector<nat>;

struct signature_entry {
    expression signature = {};
    expression_list definition = {};
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
    void define(const expression& signature, const expression_list& definition, nat back_from, nat parent = 0);
    void sort_top_by_largest();
    std::vector<std::string> llvm_key_symbols_in_table(llvm::ValueSymbolTable llvm);
};


// constants:

expression failure = {true, true, true};
expression infered_type = {{{{"__"}}}, intrin::typeless};
expression type_type = {{{{"_"}}}, intrin::typeless};
expression none_type = {{{{"_0"}}}, intrin::type};
expression unit_type = {{{{"_1"}}}, intrin::type};
expression unit_value = {{}, intrin::unit};
expression llvm_type = {{{{"_llvm"}}}, intrin::typeless};    // identical to type_type, but simply a placeholder for me.
expression application_type = {{{{"_a"}}}, intrin::type};
expression abstraction_type = {{{{"_b"}}}, intrin::type};
expression define_abstraction = {
    {
        {{"_c"}}, {{intrin::abstraction}}, // signature
        {{intrin::type}},           // of type
        {{intrin::application}}, // as definition
        {{intrin::application}}, // into scope
    }, intrin::unit};

std::vector<expression> builtins = { type_type, infered_type, none_type, unit_type, unit_value, llvm_type, application_type, abstraction_type, define_abstraction };


// program globals:
nat spaces_count_for_indent = 4;
nat max_expression_depth = 8;
bool debug = false;

// lexer globals:
static std::string text = "";
static std::string filename = "";

static nat c = 0;
static nat line = 0;
static nat column = 0;
static lexing_state lex_state = lexing_state::indent;
static token current = {};





static inline void open_file(const char* filename, arguments& args) {
    std::ifstream stream {filename};
    if (stream.good()) {
        std::string text {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        stream.close();
        args.files.push_back({filename, text});
    } else {
        printf("Unable to open \"%s\" \n", filename);
        perror("open");
        exit(1);
    }
}

static inline void print_usage() {
    printf("./nostril -[uv] [run] -[sdoei]\n./nostril -version\n./nostril run ...\n");
    exit(0);
}

static inline void print_version() {
    printf("n3zqx2l: 0.0.01\nnostril: 0.0.01\n");
    exit(0);
}

static inline void debug_arguments(const arguments& args) {
    std::cout << "file count = " <<  args.files.size() << "\n";
    for (auto a : args.files) {
        std::cout << "file: " << a.name << "\n";
        std::cout << "data: \n:::" << a.text << ":::\n";
    }
    
    std::cout << std::boolalpha;
    std::cout << "error = " << args.error << std::endl;
    std::cout << "use interpreter = " << args.interpret << std::endl;
    std::cout << "exec name = " << args.executable_name << std::endl;
}

static inline arguments get_commandline_arguments(const int argc, const char** argv) {
    arguments args = {};
    if (argc == 1) exit(1);
    const auto first = std::string(argv[1]);
    if (first == "-u") print_usage();
    else if (first == "-v") print_version();
    else if (first == "run") args.interpret = true;
    
    for (nat i = 1 + args.interpret; i < argc; i++) {
        const auto word = std::string(argv[i]);
        if (word == "-s") debug = true;
        else if (word == "-e") args.empty = true;
        else if (word == "-o" and i + 1 < argc) args.executable_name = argv[++i];
        else if (word == "-i" and i + 1 < argc) { auto n = atoi(argv[++i]); spaces_count_for_indent = n ? n : 4; }
        else if (word == "-d" and i + 1 < argc) { auto n = atoi(argv[++i]); max_expression_depth = n ? n : 4; }
        else if (word[0] == '-') { printf("bad option: %s\n", argv[i]); exit(1); }
        else open_file(argv[i], args);
    }
    if (debug) debug_arguments(args);
    return args;
}







static inline bool is_identifier(char c) { return isalnum(c) or c == '_'; }
static inline bool is_operator(char c) { return (not is_identifier(c) or not isascii(c)) and c != ' ' and c != '\t'; }

static inline bool isvalid(nat c) { return c >= 0 and c < (nat) text.size(); }

static inline void advance_by(nat n) {
    for (nat i = n; i--;) {
        if (text[c] == '\n') {
            c++; line++; column = 1;
        } else { c++; column++; }
    }
}

static inline void set_current(token_type t, lexing_state s) {
    current.type = t;
    current.value = "";
    current.line = line;
    current.column = column;
    lex_state = s;
}

static inline void check_for_lexing_errors() {
    if (state == lexing_state::string) printf("%s:%lld,%lld: unterminated string", filename, line, column);
    else if (state == lexing_state::llvm_string) print_lex_error(filename, "llvm_string", line, column);
    else if (state == lexing_state::multiline_comment) print_lex_error(filename, "multi-line comment", line, column);
}

// the main lexing function:

static inline token next() {
    while (true) {
        if (c >= (nat) text.size()) {
            check_for_lexing_errors();
            return {token_type::null, "", line, column};
        }

        if (text[c] == '\n' and lex_state == lexing_state::none) lex_state = lexing_state::indent;
        if (text[c] == ';' and isvalid(c+1) and isspace(text[c+1]) and (lex_state == lexing_state::none or lex_state == lexing_state::indent)) lex_state = lexing_state::comment;
        else if (text[c] == ';' and not isspace(text[c+1]) and (lex_state == lexing_state::none or lex_state == lexing_state::indent)) lex_state = lexing_state::multiline_comment;

        // ------------------- starting and finising ----------------------

        else if (is_identifier(text[c]) and isvalid(c+1) and not is_identifier(text[c+1]) and (lex_state == lexing_state::none or lex_state == lexing_state::indent)) {
            set_current(token_type::identifier, lexing_state::none);
            current.value = text[c];
            advance_by(1);
            clear_and_return();

        // ---------------------- starting --------------------------

        } else if (text[c] == '\"' and (lex_state == lexing_state::none or lex_state == lexing_state::indent)) { set_current(token_type::string, lexing_state::string);
        } else if (text[c] == '`' and (lex_state == lexing_state::none or lex_state == lexing_state::indent)) { set_current(token_type::llvm, lexing_state::llvm_string);
        } else if (is_identifier(text[c]) and (lex_state == lexing_state::none or lex_state == lexing_state::indent)) {
            set_current(token_type::identifier, lexing_state::identifier);
            current.value += text[c];

        // ---------------------- escaping --------------------------

        } else if (text[c] == '\\' and lex_state == lexing_state::string) {
            if (isvalid(c+1) and text[c+1] == '\"') { current.value += "\""; advance_by(1); }
            else if (isvalid(c+1) and text[c+1] == 'n') { current.value += "\n"; advance_by(1); }
            else if (isvalid(c+1) and text[c+1] == 't') { current.value += "\t"; advance_by(1); }
        //---------------------- finishing  ----------------------

        } else if ((text[c] == '\n' and lex_state == lexing_state::comment) or (text[c] == ';' and lex_state == lexing_state::multiline_comment)) {
            if (lex_state == lexing_state::comment) {
                lex_state = lexing_state::indent;
                current.type = token_type::operator_;
                current.value = "\n";
                advance_by(1);
                clear_and_return();
            }
            lex_state = lexing_state::none;

        } else if ((text[c] == '\"' and lex_state == lexing_state::string) or (text[c] == '`' and lex_state == lexing_state::llvm_string)) {
            lex_state = lexing_state::none;
            advance_by(1);
            clear_and_return();

        } else if (is_identifier(text[c]) and isvalid(c+1) and !is_identifier(text[c+1]) and lex_state == lexing_state::identifier) {
            current.value += text[c];
            lex_state = lexing_state::none;
            advance_by(1);
            clear_and_return();

        // ---------------- pushing ----------------

        } else if (lex_state == lexing_state::string or lex_state == lexing_state::llvm_string or (is_identifier(text[c]) and lex_state == lexing_state::identifier)) current.value += text[c];

        else if (lex_state == lexing_state::comment or lex_state == lexing_state::multiline_comment) {/* do nothing */}
        else if (is_operator(text[c]) and (lex_state == lexing_state::none or lex_state == lexing_state::indent)) {
            set_current(token_type::operator_, lexing_state::none);
            if (text[c] == '\n') lex_state = lexing_state::indent;
            current.value = text[c];
            advance_by(1);
            clear_and_return();

        } else if (text[c] == ' ' and lex_state == lexing_state::indent) {
            bool found_indent = true;
            for (nat i = 0; i < spaces_count_for_indent; i++) {
                if (isvalid(c+i))
                    found_indent = found_indent and text[c+i] == ' ';
                else {
                    found_indent = false;
                    break;
                }
            }
            if (found_indent) {
                current.line = line;
                current.column = column;
                current.type = token_type::indent;
                current.value = " ";
                advance_by(spaces_count_for_indent);
                clear_and_return();
            }

        } else if (text[c] == '\t' and lex_state == lexing_state::indent) {
            current.line = line;
            current.column = column;
            current.type = token_type::indent;
            current.value = " ";
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

// this function should be called before lexing a given file.
static inline void start_lex(const file& file) {
    text = file.text;
    filename = file.name;
    c = 0;
    line = 1;
    column = 1;
    lex_state = lexing_state::indent;
    current = {};
}

static inline const char* convert_token_type_representation(enum token_type type) {
    switch (type) {
        case token_type::null: return "{null}";
        case token_type::string: return "string";
        case token_type::identifier: return "identifier";
        case token_type::keyword: return "keyword";
        case token_type::operator_: return "operator";
        case token_type::character: return "character";
        case token_type::llvm: return "llvm";
        case token_type::indent: return "indent";
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




void print_expression_list(expression_list list, nat d);
void print_symbol(symbol s, nat d);
void print_expression(expression s, nat d);

void print_symbol(symbol symbol, nat d) {
    prep(d); std::cout << "symbol: \n";
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
            prep(d); std::cout << "list symbol\n";
            print_expression_list(symbol.expressions, d+1);
            break;

        case symbol_type::newline:
            prep(d); std::cout << "{newline}\n";
            break;
        case symbol_type::indent:
            prep(d); std::cout << "{indent}\n";
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
    prep(d); std::cout << "indent level = " << expression.indent_level << "\n";
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

static inline void print_symbol_line(symbol symbol) {

    switch (symbol.type) {
        case symbol_type::identifier:
            std::cout << symbol.identifier.name.value;
            break;

        case symbol_type::llvm_literal:
            std::cout << "\'" << symbol.llvm.literal.value << "\'";
            break;

        case symbol_type::string_literal:
            std::cout << "\"" << symbol.string.literal.value << "\"";
            break;

        case symbol_type::newline:
            std::cout << "{NEWLINE}";
            break;
        case symbol_type::indent:
            std::cout << "{INDENT}";
            break;

        case symbol_type::none:
            std::cout << "{NONE}\n";
            assert(false);
            break;
        default: break;
    }
}

static inline void print_expression_line(expression expression) {
    std::cout << "(";
    nat i = 0;
    for (auto symbol : expression.symbols) {
        print_symbol_line(symbol);
        if (i < (nat) expression.symbols.size() - 1) std::cout << " ";
        i++;
    }
    std::cout << ")";
    if (expression.type) std::cout << ": " << stringify_intrin(expression.type);
}

void print_expression_list(expression_list list, nat d) {

    prep(d); std::cout << "expression list:\n";
    prep(d); std::cout << "expression count = " << list.list.size() << "\n";
    nat i = 0;
    for (auto e : list.list) {
        prep(d+1); std::cout << "expression #" << i << "\n";
        prep(d+1); std::cout << std::boolalpha << "error: " << e.error << "\n";
        print_expression(e, d+2);
        prep(d+1); std::cout << "\n";
        i++;
    }
}

static inline void print_expression_list_line(expression_list list) {
    std::cout << "{\n";
    nat i = 0;
    for (auto e : list.list) {
        std::cout << "\t" << i << ": ";
        print_expression_line(e);
        std::cout << "\n";
        i++;
    }
    std::cout << "}\n";
}

static inline void print_translation_unit(expression_list unit, file file) {
    std::cout << "translation unit: (" << file.name << ")\n";
    print_expression_list(unit, 1);
}


symbol parse_symbol(const file& file, bool newlines_are_a_symbol);
expression parse_expression(const file& file, bool can_be_empty, bool newlines_are_a_symbol);
expression_list parse_expression_list(const file& file, bool can_be_empty);

inline static bool is_operator(const token& t) { return t.type == token_type::operator_; }
inline static bool is_identifier(const token& t) { return t.type == token_type::identifier; }
inline static bool is_close_paren(const token& t) { return is_operator(t) and t.value == ")"; }
inline static bool is_open_paren(const token& t) { return is_operator(t) and t.value == "("; }
inline static bool is_newline(const token& t) { return is_operator(t) and t.value == "\n"; }
inline static bool is_reserved_operator(token t) { return is_newline(t) or is_open_paren(t) or is_close_paren(t); }

static inline void newlines() {
    auto saved = save();
    auto t = next();
    while (is_newline(t)) {
        saved = save();
        t = next();
    }
    revert(saved);
}

static inline nat indents() {
    auto indent_count = 0;
    auto saved = save();
    auto indent = next();
    while (indent.type == token_type::indent) {
        indent_count++;
        saved = save();
        indent = next();
    }
    revert(saved);
    return indent_count;
}

static inline string_literal parse_string_literal() {
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

symbol parse_symbol(const file& file, bool newlines_are_a_symbol) {
    auto saved = save();
    auto t = next();
    if (is_open_paren(t)) {
        auto expressions = parse_expression_list(file, true);
        if (not expressions.error) {
            auto saved_t = t;
            t = next();
            expressions.starting_token = t;
            if (is_close_paren(t)) return { expressions };
            else {
                print_parse_error(file.name, saved_t.line, saved_t.column, convert_token_type_representation(t.type), t.value, "\")\" to close expression");
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

    symbol s = {};
    t = next();
    if (is_newline(t) and newlines_are_a_symbol) {
        s.type = symbol_type::newline;
        return s;
    } else revert(saved);

    t = next();
    if (t.type == token_type::indent and newlines_are_a_symbol) {
        s.type = symbol_type::indent;
        return s;
    } revert_and_return();
}

expression parse_expression(const file& file, bool can_be_empty, bool newlines_are_a_symbol) {
            
    std::vector<symbol> symbols = {};
    auto saved = save();
    auto start = next();
    revert(saved);
    
    auto symbol = parse_symbol(file, newlines_are_a_symbol);
    while (not symbol.error) {
        symbols.push_back(symbol);
        saved = save();
        symbol = parse_symbol(file, newlines_are_a_symbol);
    }
    revert(saved);
    
    expression result = {symbols};
    result.starting_token = start;
    result.error = not can_be_empty and symbols.empty();
    return result;
}

expression parse_terminated_expression(const file& file) {

    auto indent_count = indents();
    auto expression = parse_expression(file, /*can_be_empty = */true, /*newlines_are_a_symbol = */false);
    expression.indent_level = indent_count;

    auto saved = save();
    auto t = next();
    if (not is_newline(t) and not (is_close_paren(t) and expression.symbols.size())) { revert_and_return(); }
    if (is_close_paren(t) and expression.symbols.size()) revert(saved);
    return expression;
}

expression_list parse_expression_list(const file& file, bool can_be_empty) {
    newlines();
    
    auto saved = save();
    std::vector<expression> expressions = {};
    auto expression = parse_terminated_expression(file);
    while (not expression.error) {
        expressions.push_back(expression);
        saved = save();
        expression = parse_terminated_expression(file);
    }
    revert(saved);
    
    return {expressions, expressions.empty() and not can_be_empty};
}

expression_list parse(const file& file) {
    start_lex(file);

    if (debug) {
        debug_token_stream();
        start_lex(file);
    }

    auto unit = parse_expression_list(file, /*can_be_empty = */true);
    if (debug) print_translation_unit(unit, file);
    return unit;
}





static inline bool is_whitespace(const symbol& e) { return e.type == symbol_type::indent or e.type == symbol_type::newline; }

static inline void remove_whitespace_in_expressions(expression_list& list, const file& file, nat depth) {
    for (auto& expression : list.list) {
        auto& s = expression.symbols;
        if (std::find_if(s.begin(), s.end(), is_whitespace) != s.end())
            s.erase(std::remove_if(s.begin(), s.end(), is_whitespace));
        for (auto& symbol : s)
            if (symbol.type == symbol_type::subexpression) remove_whitespace_in_expressions(symbol.expressions, file, depth);
    }
}

void turn_indents_into_blocks(expression_list& list, const file& file,  nat level);

static inline void push_block_onto_list(expression_list& list, const file& file,  nat level, expression_list& new_list) {
    if (list.list.size()) {
        turn_indents_into_blocks(list, file, level + 1);
        if (new_list.list.empty()) new_list.list.push_back({{{list}}});
        else new_list.list.back().symbols.push_back({list});
    }
}

void turn_indents_into_blocks(expression_list& given, const file& file, nat level) {

    expression_list new_list {}, block {};
    for (auto& expression : given.list) {
        if (expression.symbols.empty()) continue;
        if (expression.indent_level > level) block.list.push_back(expression);
        else {
            push_block_onto_list(block, file, level, new_list);
            new_list.list.push_back(expression);
            block.list.clear();
        }
    }
    push_block_onto_list(block, file, level, new_list);
    given = new_list;
}

static inline void raise(nat& value, nat minimum) {
    if (value < minimum) value = minimum;
}

static inline void raise_indents(expression_list& list, const file& file, nat level) {
    for (auto& expression : list.list) {
        raise(expression.indent_level, level);
        for (auto& symbol : expression.symbols)
            if (symbol.type == symbol_type::subexpression)
                raise_indents(symbol.expressions, file, level + 1);
    }
}

static inline expression_list correct(expression_list unit, const file& file) {
    raise_indents(unit, file, 0);
    turn_indents_into_blocks(unit, file, 0);
    remove_whitespace_in_expressions(unit, file, 0);
    if (debug) print_translation_unit(unit, file);
    return unit;
}



static inline std::string stringify_intrin(nat i) {
    switch (i) {
        case intrin::typeless: return "typeless";
        case intrin::type: return "_";
        case intrin::infered: return "__";
        case intrin::none: return "_0";
        case intrin::unit: return "_1";
        case intrin::unit_value: return "()";
        case intrin::llvm: return "_llvm";
            
        case intrin::application: return "_a";
        case intrin::abstraction: return "_b";
        case intrin::define: return "_c";
    }
    return "{compiler error}";
}

const resolved_expression resolution_failure = {0, {}, true};
const resolved_expression resolved_unit_value = {intrin::unit_value, {}, false};

bool subexpression(const symbol& s) { return s.type == symbol_type::subexpression; }
static inline bool identifier(const symbol& s) { return s.type == symbol_type::identifier; }
static inline bool llvm_string(const symbol& s) { return s.type == symbol_type::llvm_literal; }
static inline bool string_literal(const symbol& s) { return s.type == symbol_type::string_literal; }
static inline bool parameter(const symbol &symbol) { return subexpression(symbol); }
static inline bool are_equal_identifiers(const symbol &first, const symbol &second) { return identifier(first) and identifier(second) and first.identifier.name.value == second.identifier.name.value; }
static inline bool is_donothing_call(llvm::Instruction* ins) { return ins and std::string(ins->getOpcodeName()) == "call" and std::string(ins->getOperand(0)->getName()) == "llvm.donothing"; }
static inline bool is_unreachable_instruction(llvm::Instruction* ins) { return ins and std::string(ins->getOpcodeName()) == "unreachable"; }

bool contains_final_terminator(llvm::Function* main_function) {
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

void prune_extraneous_subexpressions(expression_list& given);
static inline void prune_extraneous_subexpressions_in_expression(expression& given) {
    while (given.symbols.size() == 1
           and subexpression(given.symbols[0])
           and given.symbols[0].expressions.list.size() == 1
           and given.symbols[0].expressions.list.back().symbols.size()) {
        auto save = given.symbols[0].expressions.list.back().symbols;
        given.symbols = save;
    }
    for (auto& symbol : given.symbols) if (subexpression(symbol)) prune_extraneous_subexpressions(symbol.expressions);
}

void prune_extraneous_subexpressions(expression_list& given) {
    for (auto& expression : given.list) prune_extraneous_subexpressions_in_expression(expression);
}

static inline void verify(const file& file, llvm_module& module, resolved_expression_list& resolved_program) {
    std::string errors = "";
    if (llvm::verifyModule(*module, &(llvm::raw_string_ostream(errors) << ""))) {
        print_error_message(file.name, errors, 0, 0);
        resolved_program.error = true;
    }
}

static inline void debug_program(llvm_module& module, const resolved_expression_list& resolved_program, state& state) {
    if (debug) {
        debug_table(state.stack);
        print_resolved_unit(resolved_program, state);
        module->print(llvm::outs(), nullptr);
    }
}

///important note about this function:
/// it leaves artifacts in the function after use, which must be removed:
/// any occurence of a unreachable statement which is directly preceeded by
/// a llvm.do_nothing() call, should be removed before execution of the function.

static inline bool parse_llvm_string_as_instruction(const std::string& given, llvm::Function*& original, state& state, llvm::SMDiagnostic& errors) {
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

static inline resolved_expression parse_llvm_string(llvm::Function*& function, const std::string& llvm_string, nat& pointer, state& state) {
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
        print_llvm_error(function_errors, state);
        print_llvm_error(instruction_errors, state);
        print_llvm_error(type_errors, state);
        return {0, {}, true};
    }
}


//llvm::Constant* create_global_constant_string(llvm::Module* module, const std::string& string) {
//    auto type = llvm::ArrayType::get(llvm::Type::getInt8Ty(module->getContext()), string.size() + 1);
//    std::vector<llvm::Constant*> characters (string.size() + 1);
//    for (nat i = 0; i < string.size(); i++) characters[i] = llvm::ConstantInt::get(llvm::Type::getInt8Ty(module->getContext()), string[i]);
//    characters[string.size()] = llvm::ConstantInt::get(llvm::Type::getInt8Ty(module->getContext()), '\0');
//    auto llvm_string = new llvm::GlobalVariable(*module, type, true, llvm::GlobalVariable::ExternalLinkage,
//                                                llvm::ConstantArray::get(type, characters), "string");
//    return llvm::ConstantExpr::getBitCast(llvm_string, llvm::Type::getInt8Ty(module->getContext())->getPointerTo());
//}

static inline std::vector<llvm::GenericValue> turn_into_value_array(const std::vector<nat>& canonical_arguments, std::unique_ptr<llvm::Module>& module) {
    std::vector<llvm::GenericValue> arguments = {};
    for (auto index : canonical_arguments)
        arguments.push_back(llvm::GenericValue {llvm::ConstantInt::get(llvm::Type::getInt32Ty(module->getContext()), index)});
    return arguments;
}

static inline void set_data_layout(llvm_module& module) {
    auto machine = llvm::EngineBuilder(llvm_module{module.get()}).setEngineKind(llvm::EngineKind::JIT).create();
    module->setDataLayout(machine->getDataLayout());
}

static inline nat evaluate(llvm_module& module, llvm::Function* function, const std::vector<nat>& args) {
    set_data_layout(module);
    auto jit = llvm::EngineBuilder(llvm_module{module.get()}).setEngineKind(llvm::EngineKind::JIT).create();
    jit->finalizeObject();
    return jit->runFunction(function, turn_into_value_array(args, module)).IntVal.getLimitedValue();
}

static inline std::vector<nat> generate_type_list(const expression_list &given, nat given_type) {
    if (given.list.empty()) return {};
    std::vector<nat> types (given.list.size() - 1, intrin::unit);
    types.push_back(given_type);
    return types;
}

static inline bool is_unit_value(const expression& expression) {
    return expression.symbols.size() == 1
    and subexpression(expression.symbols[0])
    and expression.symbols[0].expressions.list.empty();
}

static resolved_expression construct_signature(nat fdi_length, const expression& given, nat& index) {
    resolved_expression result = {intrin::typeless};
    result.signature = std::vector<symbol>(given.symbols.begin() + index, given.symbols.begin() + index + fdi_length);
    index += fdi_length;
    return result;
}

static inline bool matches(const expression& given, const expression& signature, nat given_type, std::vector<resolved_expression_list>& args, llvm::Function*& function,
                    nat& index, nat depth, nat max_depth, nat fdi_length, state& state) {
        
    if (given_type != signature.type and given.type != intrin::infered) return false;
    for (auto symbol : signature.symbols) {
        if (index >= (nat) given.symbols.size()) return false;
        if (parameter(symbol) and subexpression(given.symbols[index])) {
            auto argument = resolve_expression_list(given.symbols[index].expressions, symbol.expressions.list.back().type, function, state);
            if (argument.error) return false;
            args.push_back(argument);
            index++;
            
        } else if (parameter(symbol)) {
            auto argument = resolve(given, symbol.expressions.list.back().type, function, index, depth + 1, max_depth, fdi_length, state);
            if (argument.error) return false;
            args.push_back({{argument}});
            
        } else if (not are_equal_identifiers(symbol, given.symbols[index])) return false;
        else index++;
    }
    return true;
}
//
//static resolved_expression parse_string(const expression &given, nat &index, state &state) {
//    auto string_type = llvm::Type::getInt8PtrTy(state.data.module->getContext());
//    auto actual_type = state.stack.master[intrin::llvm].llvm_type;
//
//    if (actual_type == string_type or true ) {   ///TODO: delete the "or true".     (why is it even here?)
//        resolved_expression string {};
//        string.index = intrin::llvm;
//        string.constant = create_global_constant_string(state.data.module, given.symbols[index].string.literal.value);
//        return string;
//    } else return resolution_failure;
//}

static inline resolved_expression resolve(const expression& given, nat given_type, llvm::Function*& function,
                            nat& index, nat depth, nat max_depth, nat fdi_length,
                            state& state) {
    
    if (index >= (nat) given.symbols.size() or not given_type or depth > max_depth)
        return resolution_failure;
    
    else if (given_type == intrin::abstraction)
        return construct_signature(fdi_length, given, index);
    ///TODO: known bug: passing multiple different length signatures doesnt work with the current fdi solution.
    
    else if (llvm_string(given.symbols[index]) and given_type == intrin::unit)
        return parse_llvm_string(function, given.symbols[index].llvm.literal.value, index, state);
    
//    else if (string_literal(given.symbols[index]) and given_type == intrin::llvm)
//        parse_string(given, index, state);
//
    
    nat saved = index;
    for (auto signature_index : state.stack.top()) {
        index = saved;
        std::vector<resolved_expression_list> args = {};
        if (matches(given, state.stack.get(signature_index), given_type, args, function, index, depth, max_depth, fdi_length, state))
            return {signature_index, args, false};
    }
    return resolution_failure;
}

static inline resolved_expression resolve_expression(const expression& given, nat given_type, llvm::Function*& function, state& state) {
    
    if (is_unit_value(given) and given_type == intrin::unit) return resolved_unit_value;
    resolved_expression solution {};
    nat pointer = 0;
    for (nat max_depth = 0; max_depth <= max_expression_depth; max_depth++) {
        for (nat fdi_length = given.symbols.size(); fdi_length--;) {
            pointer = 0;
            solution = resolve(given, given_type, function, pointer, 0, max_depth, fdi_length, state);
            if (not solution.error and pointer == (nat) given.symbols.size()) break;
        }
        if (not solution.error and pointer == (nat) given.symbols.size()) break;
    }
    if (pointer < (nat) given.symbols.size()) solution.error = true;
    if (solution.error) print_unresolved_error(given, state);
    return solution;
}

static inline resolved_expression_list resolve_expression_list(const expression_list& given, nat given_type, llvm::Function*& function, state& state) {
    if (given.list.empty()) return {{resolved_unit_value}, given_type != intrin::unit};
    nat i = 0;
    auto types = generate_type_list(given, given_type);
    resolved_expression_list solutions {};
    for (auto expression : given.list)
        solutions.list.push_back(resolve_expression(expression, types[i++], function, state));
    for (auto e : solutions.list) solutions.error |= e.error;
    return solutions;
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
    
    ///KNOWN BUG: when we have no terminators in sight, this function
    /// does remove the unneccessary basic block which is put between the
    /// bits of code which should be in the same block.
}

static inline void remove_extraneous_insertion_points_in(llvm_module& module) {
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

static inline std::string expression_to_string(const expression& given, symbol_table& stack) {
    std::string result = "(";
    nat i = 0;
    for (auto symbol : given.symbols) {
        if (symbol.type == symbol_type::identifier) result += symbol.identifier.name.value;
        else if (symbol.type == symbol_type::subexpression) result += "(" + (symbol.expressions.list.size() ? expression_to_string(symbol.expressions.list.back(), stack)  : "") + ")";
        if (i++ < (nat) given.symbols.size() - 1) result += " ";
    }
    result += ")";
    if (given.type) result += " " + expression_to_string(stack.master[given.type].signature, stack);
    return result;
}

static inline void symbol_table::update(llvm::ValueSymbolTable& llvm) {
    print_warning_message(data.file.name, "unimplemented function called", 0,0);
}

static inline void symbol_table::push_new_frame() { frames.push_back({frames.back().indicies}); }
static inline void symbol_table::pop_last_frame() { frames.pop_back(); }
static inline std::vector<nat>& symbol_table::top() { return frames.back().indicies; }
static inline expression& symbol_table::get(nat index) { return master[index].signature; }

static inline void symbol_table::define(const expression& signature, const expression_list& definition, nat back_from, nat parent) {
    // unfinsihed
    print_warning_message(data.file.name, "unimplemented function called", 0,0);
    // this function should do a check for if the signature is already
    // defined in the current scope. if so, then simply overrite its data.
    
    frames[frames.size() - (++back_from)].indicies.push_back(master.size());
    master.push_back({signature, definition, parent});
    
    //we need to define it the LLVM symbol table!
    // and we need to define it of the right type, as well.
    sort_top_by_largest();
}

static inline void symbol_table::sort_top_by_largest() {
    std::stable_sort(top().begin(), top().end(), [&](nat a, nat b) {
        return get(a).symbols.size() > get(b).symbols.size();
    });
}

static inline symbol_table::symbol_table(program_data& data, const std::vector<expression>& builtins)
: data(data) {
    
    master.push_back({});               // the null entry. a type (index) of 0 means it has no type.
    for (auto signature : builtins)
        master.push_back({signature, {}, {}});
    
    std::vector<nat> compiler_intrinsics = {};
    for (nat i = 0; i < (nat) builtins.size(); i++) compiler_intrinsics.push_back(i + 1);
    frames.push_back({compiler_intrinsics});
    sort_top_by_largest();
}

static inline std::vector<std::string> symbol_table::llvm_key_symbols_in_table(llvm::ValueSymbolTable llvm) {
    std::vector<std::string> result = {};
    for (auto i = llvm.begin(); i != llvm.end(); i++)
        result.push_back(i->getKey());
    return result;
}



de  static inline llvm_module analyze(expression_list program, const file& file, llvm::LLVMContext& context) {
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    llvm::IRBuilder<> builder(context);
    program_data data {file, module.get(), builder};
    symbol_table stack {data, builtins};
    state state {stack, data};
    stack.sort_top_by_largest();
    auto main = create_main(builder, context, module);
    builder.CreateCall(llvm::Intrinsic::getDeclaration(module.get(), llvm::Intrinsic::donothing));
    prune_extraneous_subexpressions(program);
    auto resolved = resolve_expression_list(program, intrin::unit, main, state);
    remove_extraneous_insertion_points_in(module);
    move_lone_terminators_into_previous_blocks(module);
    delete_empty_blocks(module);
    if (not contains_final_terminator(main)) append_return_0_statement(builder, main, context);
    verify(file, module, resolved);
    debug_program(module, resolved, state);
    if (resolved.error) exit(10);
    else return module;
}

std::string convert_symbol_type(enum symbol_type type) {
    switch (type) {
        case symbol_type::none: return "{none}";
        case symbol_type::subexpression: return "subexpression";
        case symbol_type::string_literal: return "string literal";
        case symbol_type::llvm_literal: return "llvm literal";
        case symbol_type::identifier: return "identifier";
        case symbol_type::newline: return "newline";
        case symbol_type::indent: return "indent";
    }
}

void print_resolved_list(resolved_expression_list list, nat depth, state& state);
void print_resolved_expr(resolved_expression expr, nat depth, state& state);

void print_resolved_expr(resolved_expression expr, nat depth, state& state) {
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
        print_resolved_list(arg, depth + 2, state);
        prep(depth); std::cout << "\n";
    }
}

void print_resolved_list(resolved_expression_list list, nat depth, state& state) {
    prep(depth); std::cout << "[error = " << std::boolalpha << list.error << "]\n";
    nat i = 0;
    for (auto expr : list.list) {
        prep(depth + 1); std::cout << "expression #" << i++ << ": \n";
        print_resolved_expr(expr, depth + 2, state);
        prep(depth); std::cout << "\n";
    }
}

void print_resolved_unit(resolved_expression_list unit, state& state) {
    std::cout << "---------- printing resolved tranlation unit: ------------\n\n";
    print_resolved_list(unit, 0, state);
}

void print_nat_vector(std::vector<nat> v, bool newline) {
    std::cout << "[ ";
    for (auto e : v) {
        std::cout << e << " ";
    }
    std::cout << "]";
    if (newline) std::cout << "\n";
}

void debug_table(symbol_table table) {
    std::cout << "---- debugging stack: ----\n";
    
    std::cout << "printing frames: \n";
    for (auto i = 0; i < (nat) table.frames.size(); i++) {
        std::cout << "\t ----- FRAME # "<<i<<"---- \n\t\tidxs: { ";
        for (auto index : table.frames[i].indicies) {
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



















void initialize_llvm() {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
}

llvm_modules frontend(const arguments& arguments, llvm::LLVMContext& context) {
    llvm_modules modules = {};
    modules.reserve(arguments.files.size());
    for (auto file : arguments.files) modules.push_back(analyze(correct(parse(file), file), file, context));
    return modules;
}

llvm_module link(llvm_modules&& modules) {
    if (modules.empty()) return {};
    auto result = std::move(modules.back());
    modules.pop_back();
    for (auto& module : modules) if (llvm::Linker::linkModules(*result, std::move(module))) exit(1);
    return result;
}

void interpret(llvm_module& module, const arguments& arguments) {
    set_data_layout(module);
    auto jit = llvm::EngineBuilder(std::move(module)).setEngineKind(llvm::EngineKind::JIT).create();
    jit->finalizeObject();
    exit(jit->runFunctionAsMain(jit->FindFunctionNamed("main"), {arguments.executable_name}, nullptr));
}

void optimize(llvm_module& module) {
    // use a pass manager, and string together as many passes as possible.
}

std::string generate_object_file(llvm_module& module, const arguments& arguments) {
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(TargetTriple);
    std::string Error = "";
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    if (not Target) exit(1);
    
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto CM = llvm::Optional<llvm::CodeModel::Model>();
    auto TheTargetMachine = Target->createTargetMachine(TargetTriple, "generic", "", opt, RM, CM);
    module->setDataLayout(TheTargetMachine->createDataLayout());
    
    auto object_filename = arguments.executable_name + ".o";
    std::error_code error;
    llvm::raw_fd_ostream dest(object_filename, error, llvm::sys::fs::F_None);
    if (error) exit(1);
    
    llvm::legacy::PassManager pass;
    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::TargetMachine::CGFT_ObjectFile)) {
        std::remove(object_filename.c_str());
        exit(1);
    }
    pass.run(*module);
    dest.flush();
    return object_filename;
}

void emit_executable(const std::string& object_file, const arguments& arguments) {
    std::string link_command = "ld -macosx_version_min 10.14 -lSystem -lc -o " + arguments.executable_name + " " + object_file + " ";
    std::system(link_command.c_str());
    if (debug) printf("executable emitted: %s\n", arguments.executable_name.c_str());
    std::remove(object_file.c_str());
}

int main(const int argc, const char** argv) {
    auto args = get_commandline_arguments(argc, argv);
    initialize_llvm();
    llvm::LLVMContext context;
    auto module = link(frontend(args, context));
    optimize(module);
    if (args.interpret) interpret(module, args);
    else emit_executable(generate_object_file(module, args), args);
}
