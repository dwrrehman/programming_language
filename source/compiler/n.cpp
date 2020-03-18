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
enum constants { none, id, op, string, expr,
    action = 'x', exec = 'o', object = 'c', ir = 'i', assembly = 's',
    
    _undefined = 0,
    _init,
    _name,
    _number,
    _30, _31, _join,
    _32, _33, _34, _declare,
    
    _type,
    _0, _lazy,
    
    _1, _2, _3, _4, _define, /// define (s: name) (t: init) (d: L t) (extern: number) -> init
    
    _28, _29, _load, /// load (file: name) (t: type) -> t             eg,       (load (hello world) unit)          <-------- this expr is of type unit.            it searches for a file called "helloworld.n"
    
    _unit,
    
    _i1, _i8, _i16, _i32, _i64, _i128, _x86_mmx, _f16, _f32, _f64, _f128,
    
    _label, _metadata, _token,
    _string, ///  string -> pointer 0 i8
    
    
    
    
    _5, _6, _pointer,           /// pointer (addrspace: number) (t: type)   -> type
    
    _7, _8, _vector,            /// vector (width: number) (t: type)  -> type
    _9, _10, _scalable,         /// scalable (width: number) (t: type)   -> type
    
    _11, _12, _array,           /// array (size: number) (t: type)   -> type
    
    _13, _opaque,               /// opaque (s: name) -> type
    _14, _15, _16, _structure,  /// struct (s: name) (d: name) (extern: number) -> type
    _17, _18, _19, _packed,     /// packed (s: name) (d: name) (extern: number) -> type
    
    _20, _function_type,         /// function (type: name)
    
    _unreachable,                   /// unreachable   -> unit
    _ret_void,                      /// ret void   -> unit
    _21, _22, _ret_value,           /// ret (t: type) (v: t)   -> unit
    
    _23, _create_label,             /// label (l: name)  -> unit
    _24, _uncond_branch,            /// jump (l: name)   -> unit
    _25, _26, _27, _cond_branch,    /// br (cond: i1) (1: name) (2: name)   -> unit
    
    _intrinsic_count
};

struct file { const char* name = ""; std::string text = ""; };
struct arguments { size_t output = action; const char* name = "a.out"; std::vector<std::string> argv_for_exec = {}; };
struct lexing_state { size_t index = 0; size_t state = none; size_t line = 0; size_t column = 0; };
struct token { size_t type = none; std::string value = ""; size_t line = 0; size_t column = 0; };
struct resolved { size_t index = 0; std::vector<resolved> args = {}; bool error = false; std::vector<struct expression> name = {}; size_t number = 0; };

struct expression {
    std::vector<struct symbol> symbols = {};
    resolved type = {}; // neccessary.
    resolved me = {}; // very questionable.
    token start = {}; // very questionable.
    bool error = false;
};

struct symbol {
    size_t type = none;
    expression subexpression = {};
    token literal = {};
    bool error = false;
};

struct entry {
    expression signature = {};
    resolved definition = {};
    resolved subsitution = {}; // very questionable.
};

static inline token next(lexing_state& l, const file& file) {
    token t = {}; auto& at = l.index; auto& s = l.state;
    while (at < file.text.size()) {
        char c = file.text[at], n = at + 1 < file.text.size() ? file.text[at + 1]:0;
        if (isalnum(c) and not isalnum(n) and s == none) { at++; return { id, std::string(1, c), l.line, l.column++};
        } else if (c == '\"' and s == none) t = { s = string, "", l.line, l.column };
        else if (isalnum(c) and s == none) t = { s = id, std::string(1, c), l.line, l.column++ };
        else if (c == '\\' and s == string) {
            if (n == '\\') t.value += "\\"; else if (n == '\"') t.value += "\""; else if (n == 'n') t.value += "\n"; else if (n == 't') t.value += "\t";
            else printf("n3zqx2l: %s:%ld:%ld: error: unknown escape sequence '\\%c'\n\n", file.name, l.line, l.column, n);
            l.column++; at++;
        } else if ((c == '\"' and s == string)) { s = none; l.column++; at++; return t; }
        else if (isalnum(c) and not isalnum(n) and s == id) { t.value += c; s = none; l.column++; at++; return t; }
        else if (s == string or (isalnum(c) and s == id)) t.value += c;
        else if (not isalnum(c) and not isspace(c) and s == none) { at++; return {op, std::string(1, c), l.line, l.column++}; }
        if (c == '\n') { l.line++; l.column = 1; } else l.column++; at++;
    } if (s == string) printf("n3zqx2l: %s:%ld:%ld: error: unterminated string\n\n", file.name, l.line, l.column);
    return {none, "", l.line, l.column};
} static inline expression parse(lexing_state& state, const file& file);
static inline symbol parse_symbol(lexing_state& state, const file& file) {
    auto saved = state; auto t = next(state, file); expression e = {};
    if (t.type == op and t.value == "(") {
        if ((e = parse(state, file)).error) return {expr, e, {}, true};
        auto close = next(state, file);
        if (close.type == op and close.value == ")") return {expr, e};
        else { state = saved; printf("n3zqx2l: %s:%ld:%ld: expected \")\"\n\n", file.name, t.line, t.column); return {expr, e, {}, true}; }
    } else if (t.type == string) return {string, {}, t};
    else if (t.type == id or (t.type == op and t.value != "(" and t.value != ")")) return {id, {}, t};
    else { state = saved; return {none, {}, {}, true}; }
}

static inline expression parse(lexing_state& state, const file& file) {
    std::vector<symbol> symbols = {};
    auto saved = state; auto start = next(state, file); state = saved;
    auto symbol = parse_symbol(state, file);
    while (not symbol.error) {
        symbols.push_back(symbol); saved = state;
        symbol = parse_symbol(state, file);
    } state = saved; return {symbols};
}

static inline std::string expression_to_string(const expression& given, const std::vector<entry>& entries, long begin = 0, long end = -1, std::vector<resolved> args = {}) {
    std::string result = "(";
    long i = 0, j = 0;
    for (auto symbol : given.symbols) {
        if (i < begin or (end != -1 and i >= end)) {
            i++;
            continue;
        }
        if (symbol.type == id) result += symbol.literal.value;
        else if (symbol.type == string) result += "\"" + symbol.literal.value + "\"";
        else if (symbol.type == expr and args.empty()) result += "(" + expression_to_string(symbol.subexpression, entries) + ")";
        else if (symbol.type == expr) {
            result += "(" + expression_to_string(entries[args[j].index].signature, entries, 0, -1, args) + ")";
            j++;
        }
        if (i++ < (long) given.symbols.size() - 1 and not (i + 1 < begin or (end != -1 and i + 1 >= end))) result += " ";
    }
    result += ")";
    if (given.type.index) result += " " + expression_to_string(entries[given.type.index].signature, entries, 0, -1, given.type.args);
    return result;
}

static inline void print_resolved_expr(resolved expr, size_t depth, std::vector<entry> entries, size_t d = 0);

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
        if (entry.subsitution.index) std::cout << " ---> " << std::to_string(entry.subsitution.index);
        if (entry.definition.index) {
            std::cout << " := \n";
            print_resolved_expr(entry.definition, 1, entries);
        }
        std::cout << "\n\n";
        j++;
    } std::cout << "}\n";
}

static inline void print_expression(expression e, size_t d);

#define prep(x)   for (size_t i = 0; i < x; i++) std::cout << ".   "

const char* convert_token_type_representation( size_t type) {
    switch (type) {
        case none: return "{none}";
        case string: return "string";
        case id: return "identifier";
        case op: return "operator";
        case expr: return "subexpr";
        default: return "INTERNAL ERROR";
    }
}

static inline void print_symbol(symbol symbol, size_t d) {
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

static inline void print_expression(expression expression, size_t d) {
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

static inline void print_resolved_expr(resolved expr, size_t depth, std::vector<entry> entries, size_t d) {
    prep(depth); std::cout << d << ": [error = " << std::boolalpha << expr.error << "]\n";
    prep(depth); std::cout << "index = " << expr.index << " :: " << expression_to_string(entries[expr.index].signature, entries, 0) << "\n";
    
    if (expr.name.size()) {
        prep(depth); std::cout << "expr = " << expression_to_string(expr.name.front(), entries) << "\n";
    }
    prep(depth); std::cout << "number = " << expr.number << "\n";
    
    std::cout << "\n";
    long i = 0;
    for (auto arg : expr.args) {
        prep(depth + 1); std::cout << "argument #" << i++ << ": \n";
        print_resolved_expr(arg, depth + 2, entries, d + 1);
        prep(depth); std::cout << "\n";
    }
}

static inline size_t define(expression signature, const resolved& definition, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack) {
    auto index = entries.size(); stack.back().push_back(index);
    signature.me.index = index; entries.push_back({signature, definition});
    std::stable_sort(stack.back().begin(), stack.back().end(), [&](size_t a, size_t b) { return entries[a].signature.symbols.size() > entries[b].signature.symbols.size(); });
    std::stable_sort(stack.back().begin(), stack.back().end(), [&](size_t a, size_t b) { return entries[a].signature.symbols.size() and entries[a].signature.symbols.front().type == expr; });
    return index;
}

static inline bool is_intrin(size_t _class, size_t to_test, const std::vector<std::vector<size_t>>& intrinsics) {
    return std::find(intrinsics[_class].begin(), intrinsics[_class].end(), to_test) != intrinsics[_class].end();
}

static inline bool equal(resolved a, resolved b, std::vector<entry>& entries) {
    if (entries[a.index].subsitution.index and equal(b, entries[a.index].subsitution, entries)) return true;
    else if (entries[b.index].subsitution.index and equal(a, entries[b.index].subsitution, entries)) return true;
    else if (entries[b.index].subsitution.index and entries[a.index].subsitution.index and equal(entries[a.index].subsitution, entries[b.index].subsitution, entries)) return true;
    else if (a.index != b.index or a.args.size() != b.args.size()) return false;
    for (unsigned long i = 0; i < a.args.size(); i++) if (not equal(a.args[i], b.args[i], entries)) return false;
    return true;
}

static inline resolved resolve(const expression& given, const resolved& given_type, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, size_t max_depth);

static expression typify(const expression& given, const resolved& initial_type, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, size_t max_depth) {
    if (given.symbols.empty()) return {{}, {}, {}, {}, true};
    expression signature = given.symbols.front().subexpression;
    for (auto& s : signature.symbols) {
        if (s.type == expr) {
            s.subexpression = typify(s.subexpression, {0}, entries, stack, intrinsics, file, max_depth);
            auto k = define(s.subexpression, {}, entries, stack);
            s.subexpression.me.index = k;
            
            ///TODO: we havent realized the right way to do this yet.
            /// it is much more beautiful, and recursive, and parsimonious.
            ///TODO: Figure it out.
        }
    }
    signature.type = initial_type;
    for (size_t i = given.symbols.size(); i-- > 1;) signature.type = resolve(given.symbols[i].subexpression, signature.type, entries, stack, intrinsics, file, max_depth);
    return signature;
}
static inline resolved construct_signature(const expression& given, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, size_t max_depth, size_t its_index) {
    ///TODO: this does unconditional succeeding. write out the logic, long hand, and make typify possibly return an error. very important.
    
    return {its_index, {}, false, {given.symbols.size() and given.symbols.front().type == expr ? typify(given, {0}, entries, stack, intrinsics, file, max_depth) : expression {given.symbols, {_undefined}}}}; ///TODO: _undefined needs to actually be "_infered". eventually.
}

static inline resolved construct_number(const expression& given, const file& file, size_t& i, size_t its_index) {
    const char* string = given.symbols[i].literal.value.c_str();
    char* pointer = NULL;
    unsigned long long n = strtoull(string, &pointer, 10);
    resolved r {its_index, {}, strlen(string) + string != pointer, {}, n};
    if (r.error) {
        printf("n3zqx2l: %s:%ld:%ld: error: expected natural number, received \"%s\"\n\n", file.name, given.symbols[i].literal.line, given.symbols[i].literal.column, string);
        return r;
    } else { i++; return r; }
}

static inline std::string convert_to_filename(const expression& e) {
    std::string result = "";
    for (auto s : e.symbols) {
        result += s.literal.value;
    } result += ".n";
    return result;
}

resolved load_file(std::vector<resolved>& args, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, size_t max_depth) {
    auto filename = convert_to_filename(args[0].name[0]);
    std::ifstream stream {filename};
    if (not stream.good()) {
        printf("n: %s: error: could not open input file: %s\n", filename.c_str(), strerror(errno));
        return {0, {}, true};
    }
    const file file = {filename.c_str(), {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()}};
    lexing_state state {0, none, 1, 1};
    return resolve(parse(state, file), args[1], entries, stack, intrinsics, file, max_depth);
}

bool debug = true;

static inline resolved resolve_at(const expression& given, const resolved& expected, size_t& i, size_t& best, size_t depth, size_t max_depth, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file) {
    if (depth > max_depth) return {0, {}, true};
    else if (i < given.symbols.size() and given.symbols[i].type == expr and is_intrin(_name, expected.index, intrinsics)) return construct_signature(given.symbols[i++].subexpression, entries, stack, intrinsics, file, max_depth, expected.index);
    else if (i < given.symbols.size() and given.symbols[i].type == expr) return resolve(given.symbols[i++].subexpression, expected, entries, stack, intrinsics, file, max_depth);
    else if (i < given.symbols.size() and given.symbols[i].type == id and is_intrin(_number, expected.index, intrinsics)) return construct_number(given, file, i, expected.index);
    else if (i < given.symbols.size() and given.symbols[i].type == string) return {_string, {}, false, {{{given.symbols[i++]}}}}; /*TODO: should expect type: "(pointer (i8))"     ie, check given_type.args[]... */
    //    else if (expected.index == _lazy) return resolve_at(given, expected.args[0], i, best, depth, max_depth, entries, stack, file);
    auto saved = i; auto saved_stack = stack;
    for (const auto s : saved_stack.back()) {
        best = std::max(i, best); i = saved; stack = saved_stack; std::vector<resolved> args = {};
        for (size_t j = 0; j < entries[s].signature.symbols.size(); j++) {
            const auto& symbol = entries[s].signature.symbols[j];
            if (i >= given.symbols.size()) { if (args.size() and j == 1) return args[0]; else goto next; }
            if (symbol.type == expr) {
                resolved argument = resolve_at(given, symbol.subexpression.type, i, best, depth + 1, max_depth, entries, stack, intrinsics, file);
                if (argument.error) goto next; args.push_back({argument});
                if (not symbol.subexpression.me.index) abort(); ///DEBUG: leave until we know this doesnt execute.
                entries[symbol.subexpression.me.index].subsitution = argument;
            } else if (symbol.type != given.symbols[i].type or symbol.literal.value != given.symbols[i].literal.value) goto next; else i++;
        }
        if (not equal(expected, entries[s].signature.type, entries)) continue;
        //        if (s == _push) stack.push_back(stack.back());
        //        if (s == _pop) stack.pop_back();
        if (is_intrin(_declare, s, intrinsics) and args[1].number < _intrinsic_count) intrinsics[args[1].number].push_back(args[0].name[0].me.index = define(args[0].name[0], {}, entries, stack));
        if (is_intrin(_define, s, intrinsics)) args[0].name[0].me.index = define(args[0].name[0],  args[2], entries, stack);
        if (is_intrin(_load, s, intrinsics)) return load_file(args, entries, stack, intrinsics, max_depth);
        return {s, args}; next: continue;
    } return {0, {}, true};
}
static inline resolved resolve(const expression& given, const resolved& given_type, std::vector<entry>& entries, std::vector<std::vector<size_t>>& stack, std::vector<std::vector<size_t>>& intrinsics, const file& file, size_t max_depth) {
    //printf("-------------- parse tree: -------------------\n");
    //print_expression(given, 0);
    size_t index = 0, best = 0;
    resolved solution = resolve_at(given, given_type, index, best, 0, max_depth, entries, stack, intrinsics, file);
    if (index < given.symbols.size()) solution.error = true;
    if (solution.error) {
        const auto b = best < given.symbols.size() ? (given.symbols[best].type == expr ? given.symbols[best].subexpression.start : given.symbols[best].literal) : given.start;
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
    
    const auto f = given.index;
    
    //    if (f == _llvm) {
    //        if (debug)
    //            printf("NOTE: error, type does not parse. llvm type was found where it shouldnt be.\n");
    //        return nullptr;
    //
    if (f == _ret_void) {
        return builder.CreateRetVoid();
        
    } else if (f == _label) {
        
        auto label_name = expression_to_string(given.args[0].name[0], entries);
        
        auto block = llvm::BasicBlock::Create(module->getContext(), label_name, function);
        
        builder.SetInsertPoint(block);
        return block;
        
    } else if (f == _uncond_branch) {
        
        auto label_name = expression_to_string(given.args[0].name[0], entries);
        auto block = llvm::BasicBlock::Create(module->getContext(), label_name, function);
        return builder.CreateBr(block);
        
    } else if (f == _name or f == _type) {
        if (debug)
            printf("error: called _type or _name: they are unimplemented.\n");
        
    } else if (f == _define) {
        
        if (debug)
            printf("error: called _define: they are unimplemented.\n");
        
        
    } else if (f == _declare) {
        
        auto the_signature = given.args[0].name[0];
        std::string the_signature_stringified = expression_to_string(the_signature, entries);
        
        if (debug)std::cout << "--------- declaring a function: " << the_signature_stringified << " ----------\n";
        
        
        
        return nullptr;
        
        
        
        /// find return type from signature:
        
        //        auto ret_type = find_llvm_type(the_signature.type, module);
        //        if (not ret_type) {
        //            if (dbg)printf("note: defaulting RET type to void...\n");
        //            ret_type = llvm::Type::getVoidTy(module->getContext());
        //        }
        
        /// find argument types from signature:
        
        //        std::vector<llvm::Type*> arg_type = {};
        //        for (auto s : the_signature.symbols) {
        //            auto t = find_llvm_type(s.subexpression.type, module);
        //            if (t) arg_type.push_back(t);
        //            else {
        //                if (dbg)printf("error: null type found in argument list\n");
        //            }
        //        }
        
        /// construct function type:
        
        //        auto function_type = llvm::FunctionType::get(ret_type, arg_type, false);
        //
        
        /// declare function:
        
        //auto declared_function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, the_signature_stringified, module);
        
        //        if (dbg){
        //            printf(" ----> just declared the following function: \n");
        //            declared_function->print(llvm::outs());        printf("\n\n");
        //        }
        
        //        if (the_signature.symbols.front().type != expr) {
        //            auto external_declared_function = llvm::Function::Create(function_type, llvm::Function::ExternalLinkage, the_signature.symbols.front().literal.value, module);
        //
        //            if (dbg) {
        //            printf(" ----> just EXTERNALLY declared the following function: \n");
        //            external_declared_function->print(llvm::outs());        printf("\n\n");
        //            }
        //        }
        
        //        return declared_function;
        
    } else {
        std::vector<llvm::Value*> arguments = {};
        
        if (f == _0) {
            
            if (debug)printf("found an EXTERNAL CALL intrinsic call: \n");
            
            if (debug)printf("the given resolved is: \n");
            if (debug)print_resolved_expr(given, 0, entries);
            if (debug)printf("\n");
            
            if (debug)printf("the signature that was picked was: \n");
//            const auto& sig = entries[given.args[1].index].signature;
//            if (debug)print_expression(sig, 0);
            if (debug)printf("\n");
            
//            if (sig.symbols.size()) {
//                auto s = sig.symbols.front();
//                if (s.type == id) {
//                    auto callee_name = s.literal.value;
//                    if (debug)std::cout << "\tfinding external function named: " << callee_name << "\n";
//                    llvm::Value* callee = module->getFunction(callee_name);
//                    if (not callee) {
//                        if (debug)printf("\tERROR: could not resolve the function at all!\n");
//                    } else {
//                        if (debug)printf("success on external function call!\n");
//
//                        for (auto arg : given.args[1].args)
//                            arguments.push_back(generate_expression(arg, entries, stack, module, function, builder));
//
//                        return builder.CreateCall(callee, arguments);
//                    }
//                }
//            }
        }
        
        for (auto arg : given.args)
            arguments.push_back(generate_expression(arg, entries, stack, module, function, builder));
        
        for (auto arg : arguments) {
            if (not arg) {
                if (debug)printf("WE ARE GOING TO FAIL!\n");
            }
        }
        
        if (f == _join) {
            if (debug)printf("returning the second argument for a call to join...\n");
            return arguments[1];
        }
        
        const auto& sig = entries[given.index].signature;
        std::string callee_name = expression_to_string(sig, entries);
        if (debug)std::cout << "finding getting n3zqx2l-named function: " << callee_name << "\n";
        llvm::Value* callee = module->getFunction(callee_name);
        if (not callee) {
            if (debug)printf("ERROR: callee n3zqx2l-named function not found... typo?\n");
            
        } else {
            if (debug)printf("success on n3zqx2l function call.\n");
            return builder.CreateCall(callee, arguments);
        }
    }
    
    
    if (debug)printf("ERROR: could not code gen for: \n\n");
    
    if (debug)print_resolved_expr(given, 0, entries);
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

static void define_intrinsic(std::string expression, std::vector<entry> &entries, const file &file, std::vector<std::vector<size_t> > &intrinsics, size_t max_depth, std::vector<std::vector<size_t> > &stack) {
    lexing_state s0 {0, none, 1, 1};
    auto s = construct_signature(parse(s0, {"_intrinsic.n", expression}), entries, stack, intrinsics, file, max_depth, _name);
    define(s.name[0], {}, entries, stack);
}

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
                std::vector<std::string> defined_intrinsics { "(i)", "(name) (i)", "(nat) (i)", "(join ((join-first) (i)) ((join-second) (i))) (i)", "(decl ((decl-name) (name) (i)) ((decl-ii) (nat) (i)) ((decl-extern) (nat) (i))) (i)"};
                std::vector<entry> entries {{}};
                std::vector<std::vector<size_t>> stack {{}}, intrinsics(_intrinsic_count, std::vector<size_t>{});
                for (size_t i = _undefined; i < _type; i++) intrinsics[i].push_back(i);
                for (auto s : defined_intrinsics) define_intrinsic(s, entries, file, intrinsics, max_depth, stack);
                
//                debug_stack(entries, stack);
//                abort();
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
