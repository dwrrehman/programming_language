#include "symbol_table.hpp"
#include "error.hpp"

#include "llvm/IR/ValueSymbolTable.h"

std::string expression_to_string(const expression& given, symbol_table& stack) {
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

void symbol_table::update(llvm::ValueSymbolTable& llvm) {
    print_warning_message(data.file.name, "unimplemented function called", 0,0);
}

void symbol_table::push_new_frame() { frames.push_back({frames.back().indicies}); }
void symbol_table::pop_last_frame() { frames.pop_back(); }
std::vector<nat>& symbol_table::top() { return frames.back().indicies; }
expression& symbol_table::get(nat index) { return master[index].signature; }

void symbol_table::define(const expression& signature, const expression_list& definition, nat back_from, nat parent) {
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

void symbol_table::sort_top_by_largest() {
    std::stable_sort(top().begin(), top().end(), [&](nat a, nat b) {
        return get(a).symbols.size() > get(b).symbols.size();
    });
}

symbol_table::symbol_table(program_data& data, const std::vector<expression>& builtins)
: data(data) {
    
    master.push_back({});               // the null entry. a type (index) of 0 means it has no type.
    for (auto signature : builtins)
        master.push_back({signature, {}, {}});
    
    std::vector<nat> compiler_intrinsics = {};
    for (nat i = 0; i < (nat) builtins.size(); i++) compiler_intrinsics.push_back(i + 1);
    frames.push_back({compiler_intrinsics});
    sort_top_by_largest();
}

std::vector<std::string> symbol_table::llvm_key_symbols_in_table(llvm::ValueSymbolTable llvm) {
    std::vector<std::string> result = {};
    for (auto i = llvm.begin(); i != llvm.end(); i++)
        result.push_back(i->getKey());
    return result;
}
