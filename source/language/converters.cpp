//
//  converters.cpp
//  language
//
//  Created by Daniel Rehman on 1908213.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "converters.hpp"

#include "nodes.hpp"
#include "lexer.hpp"
#include "builtins.hpp"
#include "analysis_ds.hpp"
#include "converters.hpp"
#include "helpers.hpp"
#include "symbol_table.hpp"

#include <string>
#include <vector>

std::string expression_to_string(const expression& given, symbol_table& stack) { 
    std::string result = "(";
    nat i = 0;
    for (auto symbol : given.symbols) { 
        if (symbol.type == symbol_type::identifier) result += symbol.identifier.name.value;
        else if (symbol.type == symbol_type::subexpression) {
            result +=
            "(" + (symbol.expressions.list.size() 
                             ? expression_to_string(symbol.expressions.list.back(), stack) 
                             : "")
            + ")";
            
        }
        if (i < given.symbols.size() - 1) result += " "; 
        i++;
    }
    result += ")";
    if (given.type) result += " " + expression_to_string(stack.master[given.type].signature, stack);
    return result;
}



//    std::cout << "STR TO EXP TAIL: received: {\n";
//    print_expression_list_line(list);
//    std::cout << "}\n";

//    // for parsing the parameter list in the main signature:
//    for (auto& element : signature.symbols) {
//        if (subexpression(element)) {
//            auto subexpressions = filter_subexpressions(element.subexpression); 
//            
//            auto result = string_to_expression_tail(subexpressions, state, flags); 
//            if (result.erroneous) state.error = true;
//            else element.subexpression = result;
//        }
//    }
////    if (signature.symbols.empty() and type == intrin::type) 
////        return intrin::unit;
//    
//
//auto result = resolved_expression;
//std::cout << "STRING TO EXPRESSION PRODUCED: ";
//print_expression(result, 0);
//std::cout << "\n";



//    std::cout << "string = \"" << given << "\"\n";




// unimpelented: WIP:

nat string_to_expression_tail(const std::vector<expression>& list, state& state) { // returns an index to the correct into the stack.
//    if (list.empty()) return intrin::infered;
//    if (list.size() == 1 and expressions_match(list[0], type_type)) return intrin::type;
    
    //auto current = list.front();
    //list.erase(list.begin());
    
    //auto resolved_expression = traverse(current, state, flags);
    ///TODO: fix me
    
    //if (resolved_expression.error) state.error = true;
    //resolved_expression.type = string_to_expression_tail(list, state, flags.dont_allow_undefined().not_at_top_level().not_parsing_a_type());
    
    return 0; // temp
}

expression string_to_expression(const std::string& given, state& state) {    
    struct file file = {"<llvm string symbol>", given};
    start_lex(file);
//    auto e = parse_expression(file, false, false);    
    //auto subexpressions = filter_subexpressions(e);    
    //auto signature = subexpressions.front();    
    
    //subexpressions.erase(subexpressions.begin());        
    //signature.type = string_to_expression_tail(subexpressions, state);
    
    //return signature;
    print_warning_message(state.data.file.name, "unimplemented function called", 0,0);
    return {};
}

expression convert_raw_llvm_symbol_to_expression(const std::string& id, llvm::Value* value, symbol_table& stack, program_data& data) { 
    if (id[0] == '(') {
        state state = {stack, data};
        return string_to_expression(id, state);
    } else {
        expression e {};
        e.type = intrin::typeless;
        e.symbols = {symbol{id}};       /// fix me!!!!!!!!
        return e;
    }
}
