//
//  builtins.cpp
//  language
//
//  Created by Daniel Rehman on 1905175.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "builtins.hpp"
/**


0  :        {typeless}          : 

1  :    __  infered type        : 

2  :    _   type type           : 

3  :    _0  none type           : _
4  :    _1  unit type           : _

5  :    ()  unit value          : _1
 
5  :    _a  application type    : _                       
 
6  :    _b  abstraction type    : _

7  :    _c  define signature    : _1
 
 
 _a         --->        how we pass around unevalatued code.
 
 _b         --->        how we pass around undefined signatures.


*/

expression failure = {true, true, true};
expression infered_type = {{{{"__"}}}, intrin::typeless};  
expression type_type = {{{{"_"}}}, intrin::typeless};
expression none_type = {{{{"_0"}}}, intrin::type};
expression unit_type = {{{{"_1"}}}, intrin::type};

expression unit_value = {{}, intrin::unit};

expression application_type = {{{{"_a"}}}, intrin::type};
expression abstraction_type = {{{{"_b"}}}, intrin::type};

expression define_abstraction = {
    {
        {{"_d"}}, {{intrin::abstraction}}, // signature
        {{intrin::type}}, // of type
        {{intrin::application}}, // as definition 
        {{intrin::application}}, // into scope
    }, intrin::unit};

expression expose_abstraction = {
    {
        {{"_e"}}, {{intrin::application}}, // existing signature
        // {{intrin::abstraction}}, // as alias                             // dont over compilcate it....?
        {{intrin::application}}, // from parent        
        {{intrin::application}}, // into scope
    }, intrin::unit};

expression print_abstraction = {
    {
        {{"_c"}},
        {{intrin::abstraction}}, // must be either "warning", "error", "info", or "note".
        {{intrin::llvm}} // a string literal:      i8*
    }, intrin::unit};



expression llvm_intrin = {{{{"_llvm"}}}, intrin::typeless}; // an llvm type:   such as i8* or i32, etc.


/////////////////////// test (for csr suite) abstractions //////////////////////////

expression hello_abstraction = {
    {
        {{"mystring"}}, {{intrin::llvm}},  
    }, intrin::unit}; 



expression dog_abstraction = {
    {
        {{"dog"}}, 
    }, intrin::unit};


expression A_abstraction = {
    {
        {{"A"}}, 
    }, intrin::application};

expression B_abstraction = {
    {
        {{"B"}}, 
    }, intrin::abstraction};


expression C_abstraction = {
    {
        {{"C"}}, 
    }, intrin::empty};



expression my_define_abstraction = {
    {
        {{"define"}}, {{intrin::abstraction}}, {{"and"}}, {{"poop"}}, {{intrin::abstraction}}, {{"stop"}}, {{"it"}}
    }, intrin::unit};




/// FIX ME
//expression compiletime_type = { { {"_compiletime"}, {{{},intrin::type}}},intrin::type};
//expression runtime_type = { { {"_runtime"}, {{{},intrin::type}}},intrin::type};
/// FIX ME
//expression immutable_type = { { {"_immutable"}, {{{},intrin::type}}},intrin::type};
//expression mutable_type = { { {"_mutable"}, {{{},intrin::type}}},intrin::type};


std::vector<expression> builtins =  {
     type_type, infered_type, none_type, unit_type, unit_value, llvm_intrin,
    
    application_type, abstraction_type,
    
    define_abstraction, 
    expose_abstraction,
    print_abstraction,
    
    hello_abstraction,
//    dog_abstraction, A_abstraction, B_abstraction, C_abstraction,
//    my_define_abstraction,        
};

std::string stringify_intrin(nat i) {
    switch (i) {
        case intrin::typeless: return "typeless";
        case intrin::type: return "_";
        case intrin::infered: return "__";
        case intrin::none: return "_0";
        case intrin::unit: return "_1";
        case intrin::empty: return "()";
        case intrin::llvm: return "_llvm";
            
        case intrin::application: return "_a";
        case intrin::abstraction: return "_b";
        
        case intrin::define: return "_d";        
        case intrin::expose: return "_e";
        case intrin::print: return "_c";
            
        /// case intrin::equals: return "_f";             // do we need this?
    }
    return "{compiler error}";
}
