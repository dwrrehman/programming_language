#include "converters.hpp"

#include "symbol_table.hpp"

std::string expression_to_string(const expression& given, symbol_table& stack) { 
    std::string result = "(";
    nat i = 0;
    for (auto symbol : given.symbols) { 
        if (symbol.type == symbol_type::identifier) result += symbol.identifier.name.value;
        else if (symbol.type == symbol_type::subexpression) result += "(" + (symbol.expressions.list.size() ? expression_to_string(symbol.expressions.list.back(), stack)  : "") + ")";
        if (i++ < given.symbols.size() - 1) result += " ";        
    }
    result += ")";
    if (given.type) result += " " + expression_to_string(stack.master[given.type].signature, stack);
    return result;
}
